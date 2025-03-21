#pragma once

#include "PgnManager.h"

namespace Chess
{
	class PgnFile
	{
	public:

		PgnFile();
		~PgnFile();

		void OpenFile(const std::filesystem::path& path);
		void SaveFile(const std::filesystem::path& path);

		PgnManager::FileID GetID() const;
		size_t GetSize() const;
		PgnGame& operator[] (size_t index);

		PgnFile& operator=(const PgnFile& other) = delete;
		void Clear();
		//void ClearSearchWork();

		void CreateGame(size_t index = -1); //index = -1 -> placed at the end
		void DeleteGame(size_t index);
		void MoveGame(size_t position, size_t direction);

	public:
		struct PgnPath_Hash
		{
			size_t operator() (const std::filesystem::path& path) const
			{
				std::size_t hash = 0;
				std::hash<std::string> hasher;

				auto cpath = std::filesystem::canonical(path);

				if (cpath.extension() != ".pgn")
					return hash;

				hash ^= hasher(cpath.filename().string()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

				while (cpath.has_parent_path())
				{
					// Combine the hash values (simple XOR + shift approach)
					hash ^= hasher(cpath.parent_path().string()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

					auto parent = cpath.parent_path();

					if (cpath == parent)
						break;

					cpath = parent;
				}
				return hash;
			}
		};

	private:
		//stream should be opened in binary mode
		friend std::istream& operator>>(std::istream& stream, PgnFile& f);

	private:
		PgnManager::FileID m_ID;
		std::shared_ptr<PgnManager::PgnFileData> m_Data;

		int m_AddedGamesCount = 0;		
	};
}