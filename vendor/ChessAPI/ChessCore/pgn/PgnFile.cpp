#include "PgnFile.h"

#include <cmath>
#include <fstream>
#include <mutex>

#include "FileHash.h"

static std::filesystem::path s_cachedDirectory = "Resources\\cache";

namespace Chess
{
	PgnFile::PgnFile()
	{
		m_Data = std::make_shared<Chess::PgnManager::PgnFileData>();
		m_ID = PgnManager::Get().AddFile(m_Data);
	}

	PgnFile::~PgnFile()
	{
		PgnManager::Get().RemoveFile(m_ID);
	}

	void PgnFile::OpenFile(const std::filesystem::path& path)
	{
		Clear();

		m_Data->FilePath = path;
		auto fileHash = shaFile(m_Data->FilePath);

		PgnPath_Hash hasher;
		auto hash = hasher(path);
		auto cachePath = s_cachedDirectory / std::to_string(hash);

		if (std::filesystem::exists(cachePath) && std::filesystem::is_directory(cachePath))
		{
			bool changed = false;

			{
				std::ifstream infile(cachePath / "sha256.hpgn", std::ios::binary);

				std::array<uint8_t, SHA_DIGEST_LENGTH> NFileHash;

				infile.read((char*)NFileHash.data(), SHA_DIGEST_LENGTH);

				if (NFileHash != fileHash)
					changed = true;

				infile.close();
			}

			if (!changed)
			{
				std::ifstream infile(cachePath / "thisfile.ppgn", std::ios::binary);

				infile.seekg(0, std::ios_base::end);
				std::streampos maxSize = infile.tellg();
				infile.seekg(0, std::ios_base::beg);

				char* data = new char[maxSize];
				infile.read(data, maxSize);

				infile.close();

				m_Data->DataPointers.reserve(maxSize / 8);

				for (int i = 0; i < maxSize; i += 8)
					m_Data->DataPointers.emplace_back(*(size_t*)(&data[i]));

				delete[] data;

				return;
			}
		}

		{
			std::ifstream infile(path, std::ios::binary);
			infile >> *this;
			infile.close();
		}

		std::filesystem::create_directory(cachePath);

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)m_Data->DataPointers.data(), m_Data->DataPointers.size() * 8);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "sha256.hpgn", std::ios::binary);
			outfile.write((char*)fileHash.data(), SHA_DIGEST_LENGTH);
			outfile.close();
		}
	}

	void PgnFile::SaveFile(const std::filesystem::path& path)
	{
		if (m_Data->EditedGames.empty() && path == m_Data->FilePath)
			return;

		PgnPath_Hash hasher;
		auto hash = hasher(path);
		auto cachePath = s_cachedDirectory / std::to_string(hash);

		if (m_Data->EditedGames.empty())
		{
			if (std::filesystem::exists(cachePath) && std::filesystem::is_directory(cachePath))
				std::filesystem::create_directory(cachePath);
			auto fileHash = shaFile(m_Data->FilePath);

			{
				std::ifstream source(m_Data->FilePath, std::ios::binary);
				std::ofstream destination(path, std::ios::binary);

				destination << source.rdbuf();

				source.close();
				destination.close();
			}
			{
				std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
				outfile.write((char*)m_Data->DataPointers.data(), m_Data->DataPointers.size() * 8);
				outfile.close();
			}
			{
				std::ofstream outfile(cachePath / "sha256.hpgn", std::ios::binary);
				outfile.write((char*)fileHash.data(), 32);
				outfile.close();
			}
		}

		std::vector<size_t> NDataPointers;
		NDataPointers.reserve(GetSize() + 1);
		NDataPointers.emplace_back(0);

		{
			std::ofstream outfile(s_cachedDirectory / "helper", std::ios::binary);

			for (int i = 0; i < GetSize(); ++i)
			{
				outfile.seekp(NDataPointers[NDataPointers.size() - 1]);
				auto dataToAdd = operator[] (i).GetData();
				outfile.write(dataToAdd.data(), dataToAdd.size());
				NDataPointers.emplace_back(dataToAdd.size() + NDataPointers[NDataPointers.size() - 1]);
			}

			outfile.close();
		}

		NDataPointers.pop_back();

		{
			std::ifstream source(s_cachedDirectory / "helper", std::ios::binary);
			std::ofstream destination(path, std::ios::binary);

			destination << source.rdbuf();

			source.close();
			destination.close();
		}

		m_Data->FilePath = path;
		auto fileHash = shaFile(m_Data->FilePath);
		m_Data->DataPointers = NDataPointers;
		m_AddedGamesCount = 0;

		{
			std::unique_lock<std::shared_mutex> ul(m_Data->FileMutex);
			m_Data->EditedGames.clear();
		}

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)m_Data->DataPointers.data(), m_Data->DataPointers.size() * 8);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "sha256.hpgn", std::ios::binary);
			outfile.write((char*)fileHash.data(), 32);
			outfile.close();
		}
	}

	std::istream& operator>>(std::istream& stream, PgnFile& f)
	{
		stream.seekg(0, std::ios_base::end);
		std::streampos maxSize = stream.tellg();
		stream.seekg(0, std::ios_base::beg);

		if (maxSize <= 0)
			return stream;

		size_t maxBufferSize = std::min((size_t)std::streamoff(maxSize), 100'000'000ui64);
		char* data = new char[maxBufferSize];

		size_t lastPointer = -1;

		while (true)
		{
			if (!f.m_Data->DataPointers.empty())
				lastPointer = f.m_Data->DataPointers[f.m_Data->DataPointers.size() - 1];

			stream.seekg(lastPointer + 1, std::ios_base::beg);

			size_t bufferSize = std::min(maxBufferSize, (size_t)std::streamoff(maxSize) - lastPointer - 1);

			stream.read(data, bufferSize);

			bool NotNoteOpen = true;
			bool NotPosibleLabelOpen = true;

			bool readyToAdd = true;

			for (size_t i = 0; i < bufferSize; i++)
			{
				if (readyToAdd && NotNoteOpen && NotPosibleLabelOpen && data[i] == '[')
					f.m_Data->DataPointers.emplace_back(lastPointer + 1 + i);

				if (data[i] >= 32 && data[i] <= 126)
				{
					if (NotPosibleLabelOpen)
					{
						if (data[i] == '{')
							NotNoteOpen = false;
						if (data[i] == '}')
							NotNoteOpen = true;
					}

					//if (NotNoteOpen)
					//{
					//	if (data[i] == '"')
					//		NotPosibleLabelOpen = !NotPosibleLabelOpen;
					//}

					if (data[i] != ']')
					{
						readyToAdd = true;
						continue;
					}

					readyToAdd = false;
					continue;
				}
			}

			if (lastPointer + 1 + bufferSize == (size_t)std::streamoff(maxSize))
				break;
		}

		delete[] data;

		return stream;
	}

	
	//void SearchMoves(std::vector<int>& possitiveIndexes, int& searched, const std::vector<std::string>& moves)
	//{
	//
	//	//unsafe
	//	bool wrong = true;
	//	searched = 0;
	//	if (moves.empty())
	//		return;
	//	for (int i = 0; i < GetSize(); i++)
	//	{
	//		for (int j = 0; j < m_Data->Games[i].GetMovePathbyRef().move.size() && j < moves.size(); j++)
	//		{
	//			wrong = false;
	//			if (m_Data->Games[i].GetMovePathbyRef().move[j] != moves[j])
	//			{
	//				wrong = true;
	//				break;
	//			}
	//		}
	//		if (!wrong)
	//			possitiveIndexes.push_back(i);
	//		searched += 1;
	//		wrong = true;
	//	}
	//}

	PgnManager::FileID PgnFile::GetID() const
	{
		return m_ID;
	}

	size_t PgnFile::GetSize() const
	{
		return m_Data->DataPointers.size() + m_AddedGamesCount;
	}

	PgnGame& PgnFile::operator[] (size_t index)
	{
		if (!m_Data->Games.contains(index))
		{
			if (m_Data->FilePath.empty())
			{
				__debugbreak();
				return m_Data->Games[0];
			}

			std::ifstream infile(m_Data->FilePath, std::ios::binary);

			size_t size;

			if (m_Data->DataPointers.size() == index + 1)
			{
				infile.seekg(0, std::ios_base::end);
				std::streampos maxSize = infile.tellg();
				infile.seekg(0, std::ios_base::beg);

				size = std::streamoff(maxSize) - m_Data->DataPointers[index];
			}
			else
				size = m_Data->DataPointers[index + 1] - m_Data->DataPointers[index];

			char* data = new char[size];

			infile.seekg(m_Data->DataPointers[index], std::ios_base::beg);
			infile.read(data, size);

			infile.close();

			std::string strData;

			for (int i = 0; i < size; i++)
			{
				if (data[i] == '\n')
					strData += data[i];

				if (data[i] >= 32 && data[i] <= 126)
				{
					strData += data[i];
				}
			}

			m_Data->Games[index].Parse(strData);

			delete[] data;
		}

		{
			std::shared_lock<std::shared_mutex> sl(m_Data->FileMutex);
			m_Data->GamesTimer[index] = std::chrono::high_resolution_clock::now();
		}

		return m_Data->Games[index];
	}

	void PgnFile::Clear()
	{
		std::unique_lock<std::shared_mutex> ul(m_Data->FileMutex);

		m_Data->FilePath = "";
		m_Data->GamesTimer.clear();
		m_Data->EditedGames.clear();
		m_Data->Games.clear();

		m_Data->DataPointers.clear();
		m_AddedGamesCount = 0;
	}

	void PgnFile::CreateGame(size_t index)
	{
		if (index == -1)
		{
			std::unique_lock<std::shared_mutex> ul(m_Data->FileMutex);
			m_Data->Games[GetSize()];
			m_Data->EditedGames[GetSize()] = 0;
			m_AddedGamesCount++;

			return;
		}
	}

	void PgnFile::DeleteGame(size_t index)
	{
		//m_Games.erase(m_Games.begin() + index);
	}

	void PgnFile::MoveGame(size_t position, size_t direction)
	{
		//m_Games.insert(m_Games.begin() + direction, m_Games[position]);
		//m_Games.erase(m_Games.begin() + direction);
	}
}