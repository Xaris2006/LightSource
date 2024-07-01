#include "AppManager.h"

#include <mutex>
#include <chrono>

std::mutex addMutex;

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

				std::vector<int> endApps;

				for (int i = 0; i < m_Apps.size(); i++)
				{
					std::string cmd;
					cmd = m_Apps[i].Read();
					//std::cout << cmd << std::endl;

					if (cmd.find("End") != std::string::npos)
						endApps.emplace_back(i);
					if (cmd.find("Open") != std::string::npos)
						CreateApp("");
				}
				
				if (m_AddApp)
				{
					addMutex.lock();

					m_AddApp = false;
					m_Apps.emplace_back(L"LightSourceApp\\LightSource.exe", std::wstring(m_cmd.begin(), m_cmd.end()));

					addMutex.unlock();
				}

				using namespace std::chrono_literals;
				std::this_thread::sleep_for(300ms);

				for (int i = 0; i < endApps.size(); i++)
				{
					m_Apps[endApps[i] - i].EndProcess();
					m_Apps.erase(m_Apps.begin() + endApps[i] - i);
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