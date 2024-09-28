#pragma once

#include <vector>
#include <filesystem>
#include <thread>


#ifndef WL_DIST

#define MYDOCUMENTS std::filesystem::current_path().parent_path() / "LightSource-Lobby\\LightSourceApp\\MyDocuments"

#else

#define MYDOCUMENTS std::filesystem::current_path().parent_path().parent_path()

#endif // !WL_DIST

namespace Tools::COBCreator
{
	class Creator
	{
	public:
		enum Status
		{
			Nothing = 0,
			Building,
			Paused
		};

	public:
		static void Init();
		static void ShutDown();
		static void OnUpdate();
		static Creator& Get();

	public:
		void AddFile(const std::filesystem::path& path);
		void RemoveFile(int index);
		void Clear();
		size_t GetSize() const;
		size_t GetFileSize(int index) const;
		std::vector<std::filesystem::path> GetFiles() const;

		void StartBuild();
		void PauseBuild();
		void ResumeBuild();
		void EndBuild();

		Status GetStatus() const;
		float GetPercentage() const;

		void SetMaxMoves(int maxMoves);
		int GetMaxMoves() const;

		void SetName(const std::string& name);
		std::string& GetName();

	private:
		Creator() = default;
		~Creator();

	private:
		std::vector<std::filesystem::path> m_files;

		Status m_status = Nothing;
		float m_percentage = 0;

		std::vector<int> m_Sizes;
		int m_allSize = 0;
		int m_maxMoves = 44;

		std::string m_name = "";

		std::thread* m_buildThread = nullptr;
		int m_controrThread = 2;
		bool m_threadEnded = true;
	};
}