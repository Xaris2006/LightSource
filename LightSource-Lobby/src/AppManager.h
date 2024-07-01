#pragma once

#include "windowsMain.h"

#include <string>
#include <vector>
#include <thread>
#include <atomic>

class AppManager
{
public:
	AppManager();
	~AppManager();

	void CreateApp(std::string cmd);
	

private:
	std::vector<Process> m_Apps;
	std::thread* m_CheckingThread = nullptr;
	std::atomic<bool> m_EndThread = false;

	HANDLE m_ChildStd_OUT_Rd = nullptr;
	HANDLE m_ChildStd_OUT_Wr = nullptr;

	bool m_AddApp = false;
	std::string m_cmd;
};