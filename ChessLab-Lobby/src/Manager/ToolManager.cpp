#include "ToolManager.h"

//#include "Walnut/Core/Assert.h"

static Manager::ToolManager* s_ToolManager = nullptr;

namespace Manager
{
	ToolManager& ToolManager::Get()
	{
		return *s_ToolManager;
	}

	void ToolManager::Init()
	{
		s_ToolManager = new ToolManager();

		s_ToolManager->m_EndThread = false;
		s_ToolManager->m_CheckingThread = new std::thread(
			[]()
			{
				while (!s_ToolManager->m_EndThread)
				{
					for (auto& [key, value] : s_ToolManager->m_Tools)
					{
						if (value.m_opened)
						{
							if (!value.m_program->IsProcessActive())
								s_ToolManager->ShutdownTool(key);
						}
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(300));
				}
			}
		);
	}

	void ToolManager::Shutdown()
	{
		s_ToolManager->m_EndThread = true;
		s_ToolManager->m_CheckingThread->join();
		delete s_ToolManager->m_CheckingThread;
		s_ToolManager->m_CheckingThread = nullptr;

		for (auto& [key, value] : s_ToolManager->m_Tools)
		{
			if (value.m_opened)
				s_ToolManager->ShutdownTool(key);
		}

		delete s_ToolManager;
		s_ToolManager = nullptr;
	}

	void ToolManager::AddTool(const std::filesystem::path& path, const std::string& name)
	{
		m_Tools[name].m_exePath = path;
	}

	void ToolManager::RemoveTool(const std::string& name)
	{
		m_Tools.erase(name);
	}
	
	void ToolManager::RunTool(const std::string& name)
	{
		m_Tools[name].m_program = new Process(m_Tools[name].m_exePath.wstring(), L"");
		m_Tools[name].m_program->Write("Ok");
		m_Tools[name].m_opened = true;
	}
	
	void ToolManager::ShutdownTool(const std::string& name)
	{
		m_Tools[name].m_program->EndProcess();
		delete m_Tools[name].m_program;
		m_Tools[name].m_program = nullptr;
		m_Tools[name].m_opened = false;
	}

	bool ToolManager::IsToolRunning(const std::string& name) const
	{
		return m_Tools.at(name).m_opened;
	}
}