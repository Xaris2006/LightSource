#pragma once

#include "../windowsMain.h"

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>


namespace Manager
{

	class AppManager
	{
	public:
		static AppManager& Get();
		static void Init();
		static void Shutdown();

	public:
		struct Command
		{
			std::string File = "";
			std::string Open = "";
			std::string Ask = "";
		};

		void CreateApp(std::string cmd);
	
		bool IsAppOpen(const std::filesystem::path& path) const;
	
	private:
		AppManager() = default;

	private:
		std::vector<Process> m_Apps;
		std::thread* m_CheckingThread = nullptr;
		std::atomic<bool> m_EndThread = false;

		std::unordered_map<int, size_t> m_OpenedPaths;
		//std::unordered_map<int, std::filesystem::path> m_OpenedPaths;
		std::vector<Command> m_Commands;

		bool m_AddApp = false;
		std::string m_cmd;
	};

}