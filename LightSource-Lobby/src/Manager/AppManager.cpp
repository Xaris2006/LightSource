#include "AppManager.h"

#include <mutex>
#include <chrono>
#include <unordered_map>
#include <filesystem>

static std::mutex addMutex;
extern bool g_AlreadyOpenedModalOpen;

static void fixPath(std::string& strpath)
{
	if (strpath.empty())
		return;

	if (strpath[0] == '\"')
		strpath.erase(0, 1);

	for (int i = 1; i < strpath.size(); i++)
	{
		if (strpath[i] == '\\' && strpath[i - 1] == '\\')
		{
			strpath.erase(i, 1);
			i--;
			continue;
		}
		if (strpath[i] == '\"')
		{
			strpath.erase(i, 1);
			i--;
			continue;
		}
	}
}

namespace Manager
{

	AppManager::AppManager()
	{
		m_CheckingThread = new std::thread(
			[this]()
			{
				while (true)
				{
					if (m_EndThread)
					{
						for (int i = 0; i < m_Apps.size(); i++)
						{
							m_Apps[i].EndProcess();
						}
						return;
					}

					m_Commands.clear();
					//m_OpenedPaths.clear();
					std::vector<int> endApps;
					std::unordered_map<int, std::filesystem::path> AskForNewFilePath;

					m_Commands.resize(m_Apps.size());

					for (int i = 0; i < m_Apps.size(); i++)
					{
						std::string Strcmd(m_Apps[i].Read());
						//std::cout << Strcmd << std::endl;

						if (Strcmd.find("*End") != std::string::npos)
							endApps.emplace_back(i);

						size_t index = 0;
						std::vector<std::string> rawCommand;
						while (index < Strcmd.size())
						{
							size_t StartIndex = Strcmd.find("*", index);
							if (StartIndex != std::string::npos)
							{
								index = Strcmd.find('\n', StartIndex);
								if (index == std::string::npos)
									break;
								rawCommand.emplace_back(Strcmd.begin() + StartIndex, Strcmd.begin() + index);
								continue;
							}
							index = Strcmd.size();
						}
						for (auto& rcmd : rawCommand)
						{
							{
								size_t FileIndex = rcmd.find("*File");
								if (FileIndex != std::string::npos)
								{
									m_Commands[i].File = std::string(rcmd.begin() + FileIndex, rcmd.end());
									continue;
								}
							}
							{
								size_t OpenIndex = rcmd.find("*Open");
								if (OpenIndex != std::string::npos)
								{
									m_Commands[i].Open = std::string(rcmd.begin() + OpenIndex, rcmd.end());
									continue;
								}
							}
							{
								size_t AskIndex = rcmd.find("*Ask");
								if (AskIndex != std::string::npos)
								{
									m_Commands[i].Ask = std::string(rcmd.begin() + AskIndex, rcmd.end());
									continue;
								}
							}
						}
					}

					for (int i = 0; i < m_Commands.size(); i++)
					{
						if (m_Commands[i].File.size())
						{
							std::string path = "";
							size_t IndexPath = m_Commands[i].File.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(m_Commands[i].File.begin() + IndexPath + 5, m_Commands[i].File.begin() + m_Commands[i].File.find(":Path"));

							fixPath(path);

							if (!path.empty())
							{
								m_OpenedPaths[i] = path;
							}
						}

						if (m_Commands[i].Open.size())
						{
							std::string path = "";
							size_t IndexPath = m_Commands[i].Open.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(m_Commands[i].Open.begin() + IndexPath + 5, m_Commands[i].Open.begin() + m_Commands[i].Open.find(":Path"));

							fixPath(path);

							CreateApp(path);
						}

						if (m_Commands[i].Ask.size())
						{
							std::string path = "";
							size_t IndexPath = m_Commands[i].Ask.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(m_Commands[i].Ask.begin() + IndexPath + 5, m_Commands[i].Ask.begin() + m_Commands[i].Ask.find(":Path"));

							fixPath(path);

							if (path.empty())
								m_Apps[i].Write("Accept\n");
							else
								AskForNewFilePath[i] = path;
						}
					}

					for (auto& [key, value] : AskForNewFilePath)
					{
						bool alreadyOpened = false;
						for (auto& [otherKey, otherValue] : m_OpenedPaths)
						{
							std::filesystem::path completeValue = std::filesystem::path(value);
							if (std::filesystem::path(value).is_relative())
								completeValue = std::filesystem::current_path() / "LightSourceApp" / value;

							if (std::filesystem::path(otherValue) == completeValue)
							{
								alreadyOpened = true;
								break;
							}
						}

						if (alreadyOpened)
							m_Apps[key].Write("Decline\n");
						else
							m_Apps[key].Write("Accept\n");
					}

					if (m_AddApp)
					{
						m_AddApp = false;
						bool alreadyOpened = false;

						std::filesystem::path npath = m_cmd;

						for (auto& [key, other] : m_OpenedPaths)
						{
							if (other == npath)
							{
								alreadyOpened = true;
								g_AlreadyOpenedModalOpen = true;
								break;
							}
						}

						if (!alreadyOpened)
						{
							addMutex.lock();

							m_Apps.emplace_back(L"LightSourceApp\\LightSource.exe", std::wstring(m_cmd.begin(), m_cmd.end()));
							m_Apps[m_Apps.size() - 1].Write("Ok");

							addMutex.unlock();
						}
					}

					using namespace std::chrono_literals;
					std::this_thread::sleep_for(300ms);

					for (int i = 0; i < endApps.size(); i++)
					{
						m_Apps[endApps[i] - i].EndProcess();
						m_Apps.erase(m_Apps.begin() + endApps[i] - i);
						m_OpenedPaths.erase(endApps[i]);
					}
				}
			}
		);
	}

	AppManager::~AppManager()
	{
		m_EndThread = true;
		m_CheckingThread->join();
		delete m_CheckingThread;
	}

	void AppManager::CreateApp(std::string cmd)
	{
		addMutex.lock();

		m_AddApp = true;
		m_cmd = cmd;

		addMutex.unlock();
	}

}