#include "Creator.h"

#include "ChessCore/Opening_Book/OpeningBook.h"
#include "ChessCore/pgn/Pgn.h"
#include "ChessCore/GameManager.h"

#include <fstream>

static Tools::COBCreator::Creator* s_creator = nullptr;

namespace Tools::COBCreator
{
	void Creator::Init()
	{
		s_creator = new	Creator();
	}

	void Creator::ShutDown()
	{
		delete s_creator;
		s_creator = nullptr;
	}

	void Creator::OnUpdate()
	{
		if (s_creator->m_buildThread && s_creator->m_threadEnded)
		{
			s_creator->EndBuild();
		}
	}

	Creator& Creator::Get()
	{
		return *s_creator;
	}

	Creator::~Creator()
	{
		EndBuild();
	}

	void Creator::AddFile(const std::filesystem::path& path)
	{
		if (m_buildThread)
			return;

		bool founded = false;

		for (auto& otherpath : m_files)
		{
			if (otherpath == path)
			{
				founded = true;
				break;
			}
		}

		if (founded)
			return;

		Chess::PgnFile pgnfile;
		pgnfile.OpenFile(path);

		if (pgnfile.GetSize() == 0)
			return;

		m_files.emplace_back(path);
		m_Sizes.emplace_back(pgnfile.GetSize());
		m_allSize += pgnfile.GetSize();
	}

	void Creator::RemoveFile(int index)
	{
		if (m_buildThread)
			return;

		m_allSize -= m_Sizes[index];

		m_Sizes.erase(m_Sizes.begin() + index);
		m_files.erase(m_files.begin() + index);
	}

	void Creator::Clear()
	{
		if (m_buildThread)
			return;

		m_files.clear();
		m_Sizes.clear();
		m_allSize = 0;
	}

	size_t Creator::GetSize() const
	{
		return m_files.size();
	}

	size_t Creator::GetFileSize(int index) const
	{
		return m_Sizes[index];
	}

	std::vector<std::filesystem::path> Creator::GetFiles() const
	{
		return m_files;
	}

	void Creator::StartBuild()
	{
		if (m_buildThread)
			return;

		m_controrThread = 0;
		m_status = Status::Building;
		m_threadEnded = false;

		m_buildThread = new std::thread(
			[&]()
			{
				std::unordered_map<Chess::OpeningBook::PositionID, std::vector<Chess::OpeningBook::MoveOB>, Chess::OpeningBook::PositionID_Hash> PositionMoveMap;

				PositionMoveMap.reserve(m_allSize / 100);

				m_percentage = 0;

				int overall = 0;

				for (auto& path : m_files)
				{
					Chess::PgnFile pgnfile;
					pgnfile.OpenFile(path);

					if (!pgnfile.GetSize()) { continue; }

					Chess::OpeningBook::PositionID pos;
					int moveIndex = -1;
					int strindex;

					Chess::GameManager game;
					game.InitPgnGame(pgnfile[0]);

					for (int i = 0; i < pgnfile.GetSize(); i++)
					{
						if (m_controrThread)
						{
							if (m_controrThread == 2)
								return;
							else
							{
								while (m_controrThread)
								{
									if (m_controrThread == 2)
										return;
									std::this_thread::sleep_for(std::chrono::milliseconds(200));
								}
							}
						}

						overall++;
						float s = (float)overall / (float)m_allSize;
						m_percentage = s * 100.0f - 1;
						if (m_percentage < 0)
							m_percentage = 0;

						if (pgnfile[i].GetResault() == "*")
							continue;

						if (pgnfile[i]["FEN"] != "?" && pgnfile[i]["FEN"] != "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
							continue;

						game.InitPgnGame(pgnfile[i]);
						std::string move = "";
						auto& movestr = game.GetPgnGame().GetMovePathbyRef();

						if (game.GetMovesByStr().move.empty())
							continue;

						for (int j = 0; j < movestr.move.size() && j < m_maxMoves; j++)
						{
							game.GoNextMove();
						}
						for (int j = movestr.move.size() - 1 < m_maxMoves - 1 ? movestr.move.size() - 1 : m_maxMoves - 1; j >= 0; j--)
						{
							if (movestr.move[j] == "child")
								continue;
							pos = game.GetFormatedFEN();
							if (move != "")
							{
								auto& mapelement = PositionMoveMap[pos];
								for (int x = 0; x < mapelement.size(); x++)
								{
									if (mapelement[x].strmove == move)
									{
										moveIndex = x;
										break;
									}
								}
								if (moveIndex == -1)
								{
									mapelement.push_back({ move });
									moveIndex = mapelement.size() - 1;
								}

								auto& cur_move = mapelement[moveIndex];
								cur_move.played += 1;
								if (pgnfile[i].GetResault() == "1-0")
									cur_move.whitewins += 1;
								else if (pgnfile[i].GetResault() == "0-1")
									cur_move.blackwins += 1;
								moveIndex = -1;
							}
							strindex = movestr.move[j].find(' ');
							move = movestr.move[j].substr(strindex + 1, movestr.move[j].size() - strindex + 1);
							game.GoPreviusMove();
						}
						pos = game.GetFormatedFEN();
						if (move != "")
						{
							auto& mapelement = PositionMoveMap[pos];
							for (int x = 0; x < mapelement.size(); x++)
							{
								if (mapelement[x].strmove == move)
								{
									moveIndex = x;
									break;
								}
							}
							if (moveIndex == -1) { mapelement.push_back({ move }); moveIndex = mapelement.size() - 1; }

							auto& cur_move = mapelement[moveIndex];
							cur_move.played += 1;
							if (pgnfile[i].GetResault() == "1-0")
								cur_move.whitewins += 1;
							else if (pgnfile[i].GetResault() == "0-1")
								cur_move.blackwins += 1;
							moveIndex = -1;
						}
					}
				}

				std::vector<uint8_t> buffer;
				buffer.reserve(PositionMoveMap.size() * 40);
				uint8_t* intptr = nullptr;

				const std::string format_version = "COB_FORMAT_VERSION_0.1";

				for (int x = 0; x < format_version.size(); x++)
					buffer.push_back(format_version[x]);


				for (auto& it = PositionMoveMap.begin(); it != PositionMoveMap.end(); it++)
				{
					for (int j = 0; j < it->first.size(); j++)
					{
						buffer.emplace_back(it->first[j]);
					}
					for (int j = 0; j < it->second.size(); j++)
					{
						auto& cur_move = it->second[j];
						for (int x = 0; x < cur_move.strmove.size(); x++)
							buffer.emplace_back(cur_move.strmove[x]);
						buffer.emplace_back(0b00000000);

						intptr = (uint8_t*)(void*)(&cur_move.whitewins);
						buffer.emplace_back(intptr[0]);
						buffer.emplace_back(intptr[1]);
						buffer.emplace_back(intptr[2]);
						buffer.emplace_back(intptr[3]);

						intptr = (uint8_t*)(void*)(&cur_move.blackwins);
						buffer.emplace_back(intptr[0]);
						buffer.emplace_back(intptr[1]);
						buffer.emplace_back(intptr[2]);
						buffer.emplace_back(intptr[3]);

						intptr = (uint8_t*)(void*)(&cur_move.played);
						buffer.emplace_back(intptr[0]);
						buffer.emplace_back(intptr[1]);
						buffer.emplace_back(intptr[2]);
						buffer.emplace_back(intptr[3]);
					}
					buffer.emplace_back(0b00000000);
				}

				if (m_controrThread)
				{
					if (m_controrThread == 2)
						return;
					else
					{
						while (m_controrThread)
						{
							if (m_controrThread == 2)
								return;
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
						}
					}
				}

				std::ofstream outfile(MYDOCUMENTS / "Books" / (m_name + ".cob"), std::ios_base::binary);
				outfile.write((const char*)buffer.data(), buffer.size());
				outfile.close();

				m_percentage = 100;
				m_threadEnded = true;
			}

		);
	}

	void Creator::PauseBuild()
	{
		if (!m_buildThread)
			return;

		m_controrThread = 1;
		m_status = Status::Paused;
	}

	void Creator::ResumeBuild()
	{
		if (!m_buildThread)
			return;

		m_controrThread = 0;
		m_status = Status::Building;
	}

	void Creator::EndBuild()
	{
		if (!m_buildThread)
			return;

		m_controrThread = 2;
		m_status = Status::Nothing;

		m_buildThread->join();
		delete m_buildThread;
		m_buildThread = nullptr;
		
		m_percentage = 0;
	}

	Creator::Status Creator::GetStatus() const
	{
		return m_status;
	}
	
	float Creator::GetPercentage() const
	{
		return m_percentage;
	}

	void Creator::SetMaxMoves(int maxMoves)
	{
		if (m_status != Nothing)
			return;
		m_maxMoves = maxMoves;
	}

	int Creator::GetMaxMoves() const
	{
		return m_maxMoves;
	}
	
	void Creator::SetName(const std::string& name)
	{
		if (m_status != Nothing)
			return;
		m_name = name;
	}

	std::string& Creator::GetName()
	{
		return m_name;
	}
}