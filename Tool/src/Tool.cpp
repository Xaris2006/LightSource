#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Tools/Engine_Manager/Engine_Manager.h"

std::string g_AppDirectory;
Walnut::ApplicationSpecification g_spec;

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	return Tools::EngineManager::CreateApplication(argc, argv);
}