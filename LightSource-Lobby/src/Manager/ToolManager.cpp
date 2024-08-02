#include "ToolManager.h"

static Manager::ToolManager* s_ToolManager = nullptr;

namespace Manager
{
	ToolManager* ToolManager::Get()
	{
		return s_ToolManager;
	}

	void ToolManager::Init()
	{
		if (s_ToolManager)
			throw std::runtime_error("ToolManager is reinitialized without it being shuted down!");
		
		s_ToolManager = new ToolManager();
	}

	void ToolManager::Shutdown()
	{
		if (!s_ToolManager)
			throw std::runtime_error("ToolManager is shuted down but is nullptr!");

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
		if (m_Tools[name].m_program)
			throw std::runtime_error("Tool already opened!");

		m_Tools[name].m_program = new Process(m_Tools[name].m_exePath.wstring(), L"");
		m_Tools[name].m_program->Write("Ok");
		m_Tools[name].m_opened = true;
	}
	
	void ToolManager::ShutdownTool(const std::string& name)
	{
		if (!m_Tools[name].m_program)
			throw std::runtime_error("Tool already closed!");

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