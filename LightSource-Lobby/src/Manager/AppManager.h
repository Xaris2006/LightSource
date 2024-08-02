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
		struct Command
		{
			std::string File = "";
			std::string Open = "";
			std::string Ask = "";
		};

	public:
		AppManager();
		~AppManager();

		void CreateApp(std::string cmd);

	private:
		std::vector<Process> m_Apps;
		std::thread* m_CheckingThread = nullptr;
		std::atomic<bool> m_EndThread = false;

		std::unordered_map<int, std::filesystem::path> m_OpenedPaths;
		std::vector<Command> m_Commands;

		bool m_AddApp = false;
		std::string m_cmd;
	};

}