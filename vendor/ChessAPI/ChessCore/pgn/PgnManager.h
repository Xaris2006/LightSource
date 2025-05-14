#pragma once

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <filesystem>

#include "PgnGame.h"

#include "UUID/UUID.h"

namespace Chess
{
	class PgnManager
	{
	public:
		static PgnManager& Get();
		static void Init();
		static void Shutdown();

	public:
		using FileID   = UUID;
		using WorkID   = UUID;
		using TargetID = UUID;

		struct PgnFileData
		{
			std::shared_mutex FileMutex;

			std::filesystem::path				FilePath = "";
			std::unordered_map<size_t, PgnGame> Games;//protect
			std::unordered_set<size_t> EditedGames;//protect
			std::unordered_map<size_t, std::chrono::high_resolution_clock::time_point> GamesTimer;//protect
			std::vector<size_t> DataPointers;
		};
		
		struct Target
		{
			std::mutex TargetMutex;

			FileID FDataID;
			std::map<std::string, std::string> Settings;
			std::vector<size_t> PossitiveIndexes;//protect
			//bool Ended = false;
			//size_t JobRemain = -1;//protect
		};

		struct SearchWorkData
		{
			static constexpr size_t s_countPerJob = 500ull;
			
			struct Job
			{
				TargetID tID;
				size_t Index = -1;
			};

		public:
			std::shared_mutex WorkMutex;

			std::unordered_map<TargetID, std::shared_ptr<Target>> Targets;
			std::vector<size_t> AllPossitiveIndexes;

			size_t Jobs = 0;
			std::list<Job> JobsToDo;//protect
		};

	public:
		FileID AddFile(std::shared_ptr<PgnFileData> ptr);
		void RemoveFile(FileID id);

		WorkID AddWork(std::shared_ptr<SearchWorkData> ptr);
		void RemoveWork(WorkID id);

		std::shared_ptr<PgnFileData> GetFileData(FileID id) const;

	private:
		PgnManager() = default;

	private:
		std::unordered_map<FileID, std::shared_ptr<PgnFileData>> m_FileDataPtr;
		std::unordered_map<WorkID, std::shared_ptr<SearchWorkData>> m_WorkDataPtr;

		std::thread* m_ThreadFileHandler = nullptr;
		bool m_endThread = true;

		static constexpr size_t s_amountOfWorkers = 4;

		std::array<std::thread*, s_amountOfWorkers> m_SearchWorkers;
		std::array<bool, s_amountOfWorkers> m_endWorkers;
	};

}