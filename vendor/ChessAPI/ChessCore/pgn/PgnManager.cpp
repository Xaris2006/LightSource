#include "PgnManager.h"

#include "FileHash.h"

static Chess::PgnManager* s_PgnManager = nullptr;

std::mutex s_FileDataAccessMutex;
std::mutex s_WorkDataAccessMutex;
std::mutex s_WorkThreadsMutex;

namespace Chess
{
	void PgnManager::Init()
	{
		s_PgnManager = new PgnManager();

		s_PgnManager->m_endThread = false;

		s_PgnManager->m_ThreadFileHandler = new std::thread(
			[]()
			{
				while (!s_PgnManager->m_endThread)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));

					s_FileDataAccessMutex.lock();

					for (auto& [id, data] : s_PgnManager->m_FileDataPtr)
					{
						std::unique_lock<std::shared_mutex> ul(data->FileMutex);

						for (auto it = data->GamesTimer.begin(); it != data->GamesTimer.end();)
						{
							auto& [key, value] = *it;

							if (data->EditedGames.contains(key))
							{
								++it;
								continue;
							}

							auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - value).count();

							bool hasRef = !data->Games[key].IsFree();

							if (time > 1000 && !hasRef)
							{
								data->Games.erase(key);
								it = data->GamesTimer.erase(it);

								continue;
							}

							if (hasRef && Sha256(data->Games[key].GetData()) != Sha256(data->Games[key].GetDataRead()))
								data->EditedGames[key] = 0;

							++it;
						}
					}

					s_FileDataAccessMutex.unlock();
				}

				return;
			});

		int numberOfActive = 0;

		for (int threadIndex = 0; threadIndex < 4; threadIndex++)
		{
			s_PgnManager->m_endWorkers[threadIndex] = false;
			
			s_PgnManager->m_SearchWorkers[threadIndex] = new std::thread(
				[threadIndex, &numberOfActive]()
				{
					std::vector<std::pair<PgnGame, size_t>> games;
					std::vector<int> IndexesToAdd;
			
					bool leader = false;

					while (!s_PgnManager->m_endWorkers[threadIndex])
					{
						games.clear();
						IndexesToAdd.clear();
						
						bool found = false;
						WorkID ID;
						SearchWorkData::Job job;

						s_WorkDataAccessMutex.lock();

						s_WorkThreadsMutex.lock();

						for (auto& [id, data] : s_PgnManager->m_WorkDataPtr)
						{
							//data->WorkMutex.lock_shared();
							std::shared_lock<std::shared_mutex> slW(data->WorkMutex);
							if (!data->JobsToDo.empty())
							{
								found = true;
								job = data->JobsToDo.front();
								data->JobsToDo.pop_front();
								ID = id;

								break;
							}

							//data->WorkMutex.unlock_shared();
						}

						s_WorkThreadsMutex.unlock();

						if (!found)
						{
							s_WorkDataAccessMutex.unlock();
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							continue;
						}

						auto&& workData = s_PgnManager->m_WorkDataPtr[ID];

						s_WorkDataAccessMutex.unlock();

						{
							std::shared_lock<std::shared_mutex> slW(workData->WorkMutex);

							if (!workData->Targets.contains(job.tID))
							{
								continue;
							}

							auto&& target = workData->Targets[job.tID];


							if (!Get().m_FileDataPtr.contains(target->FDataID))
								continue;

							std::shared_ptr<PgnFileData> fileData = Get().GetFileData(target->FDataID);

							std::shared_lock<std::shared_mutex> slF(fileData->FileMutex);

							{
								if (fileData->DataPointers.empty() || fileData->FilePath.empty())
									continue;

								std::ifstream infile(fileData->FilePath, std::ios::binary);

								size_t size;

								if (fileData->DataPointers.size() <= job.Index + workData->countPerJob)
								{
									infile.seekg(0, std::ios_base::end);
									std::streampos maxSize = infile.tellg();
									infile.seekg(0, std::ios_base::beg);

									size = std::streamoff(maxSize) - fileData->DataPointers[job.Index];
								}
								else
									size = fileData->DataPointers[job.Index + workData->countPerJob] - fileData->DataPointers[job.Index] - 1;

								char* data = new char[size];

								infile.seekg(fileData->DataPointers[job.Index], std::ios_base::beg);
								infile.read(data, size);

								infile.close();

								std::string strData;
								size_t nextMax = size;

								size_t pointerMax = std::min(fileData->DataPointers.size(), job.Index + workData->countPerJob);

								for (int j = job.Index; j < pointerMax; j++)
								{
									if (fileData->EditedGames.contains(j))
										continue;

									strData.clear();
									nextMax = size;

									if (fileData->DataPointers.size() > j + 1)
										nextMax = fileData->DataPointers[j + 1] - fileData->DataPointers[job.Index];

									for (int o = fileData->DataPointers[j] - fileData->DataPointers[job.Index]; o < nextMax; o++)
									{
										if (data[o] == '\n')
											strData += data[o];

										if (data[o] >= 32 && data[o] <= 126)
											strData += data[o];
									}

									games.emplace_back(PgnGame(), j).first.Parse(strData);
								}

								delete[] data;
							}

							bool doubleBreak = false;
							int correct = 0;
							for (int j = 0; j < games.size(); j++)
							{
								if (fileData->EditedGames.contains(games[j].second))
									continue;

								for (auto& existedLabelName : games[j].first.GetLabelNames())
								{
									for (auto& [searchLabelName, LabelValue] : target->Settings)
									{
										if (existedLabelName == searchLabelName)
										{
											if (games[j].first[searchLabelName].find(LabelValue) + 1)
												correct += 1;
											else
												doubleBreak = true;
											break;
										}
									}
									if (doubleBreak)
									{
										doubleBreak = false;
										break;
									}
									if (correct == target->Settings.size())
									{
										IndexesToAdd.emplace_back(games[j].second);
										break;
									}
								}
								correct = 0;
								//searched += 1;
							}

							target->TargetMutex.lock();

							for (int j = 0; j < IndexesToAdd.size(); j++)
							{
								target->PossitiveIndexes.emplace_back(IndexesToAdd[j]);
								workData->AllPossitiveIndexes.emplace_back(IndexesToAdd[j]);
							}

							target->TargetMutex.unlock();
						}
					}
			
					return;
				});
		}
	}

	void PgnManager::Shutdown()
	{
		s_PgnManager->m_endWorkers.fill(true);

		for (int i = 0; i < 4; i++)
		{
			s_PgnManager->m_SearchWorkers[i]->join();
			delete s_PgnManager->m_SearchWorkers[i];
		}

		s_PgnManager->m_endThread = true;
		s_PgnManager->m_ThreadFileHandler->join();
		delete s_PgnManager->m_ThreadFileHandler;
		s_PgnManager->m_ThreadFileHandler = nullptr;

		//delete s_PgnManager;
		//s_PgnManager = nullptr;
	}

	PgnManager& PgnManager::Get()
	{
		return *s_PgnManager;
	}

	PgnManager::FileID PgnManager::AddFile(std::shared_ptr<PgnFileData> ptr)
	{
		FileID nID = UUID();
		s_FileDataAccessMutex.lock();
		m_FileDataPtr[nID] = ptr;
		s_FileDataAccessMutex.unlock();

		return nID;
	}

	void PgnManager::RemoveFile(PgnManager::FileID id)
	{
		s_FileDataAccessMutex.lock();

		m_FileDataPtr.erase(id);

		s_FileDataAccessMutex.unlock();
	}

	PgnManager::WorkID PgnManager::AddWork(std::shared_ptr<SearchWorkData> ptr)
	{
		FileID nID = UUID();
		s_WorkDataAccessMutex.lock();
		m_WorkDataPtr[nID] = ptr;
		s_WorkDataAccessMutex.unlock();

		return nID;
	}

	void PgnManager::RemoveWork(PgnManager::WorkID id)
	{
		s_WorkDataAccessMutex.lock();

		m_WorkDataPtr.erase(id);

		s_WorkDataAccessMutex.unlock();
	}

	std::shared_ptr<PgnManager::PgnFileData> PgnManager::GetFileData(FileID id) const
	{
		return m_FileDataPtr.at(id);
	}
}