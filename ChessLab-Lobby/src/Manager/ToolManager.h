#pragma once

#include "../windowsMain.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace Manager
{
	struct Tool
	{
		bool m_opened = false;
		std::filesystem::path m_exePath = "";
		Process* m_program = nullptr;
	};


	class ToolManager
	{
	public:
		static ToolManager& Get();
		static void Init();
		static void Shutdown();

	public:
		void AddTool(const std::filesystem::path& path, const std::string& name);
		void RemoveTool(const std::string& name);

		void RunTool(const std::string& name);
		void ShutdownTool(const std::string& name);
		bool IsToolRunning(const std::string& name) const;

	private:
		ToolManager() = default;

	private:
		std::thread* m_CheckingThread = nullptr;
		std::atomic<bool> m_EndThread = false;

		std::unordered_map<std::string, Tool> m_Tools;
	};
}