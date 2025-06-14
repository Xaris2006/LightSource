#pragma once

#include <string>
#include <thread>
#include <atomic>

class Update
{
public:
	Update();

	void OnImGuiRender();

	void ShowUpdateModal();

	std::string GetVersion() const;
	std::string GetLatestVersion() const;

	bool IsUpdateAvailable() const;

private:
	void StartOperationForUpdate();

	void UI_DrawUpdateModal();
	void UI_DrawStartUpdatingModal();

	void ShowStartUpdatingModal();

private:
	std::string m_Version;
	std::string m_LatestVersion;
	bool m_UpdateAvailable = false;

	bool m_UpdateModalOpen = false;
	bool m_StartUpdatingModalOpen = false;
	
	std::thread* m_Updater = nullptr;
	std::atomic<bool> m_ThreadEnded = false;
	std::atomic<bool> m_UpdateFailed = false;
};