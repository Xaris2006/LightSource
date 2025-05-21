#include "Update.h"

#include <fstream>
#include "Web.h"

#include "imgui.h"
#include "Walnut/ApplicationGUI.h"
#include "Walnut/ImGui/ImGuiTheme.h"
#include "Walnut/UI/UI.h"

#include <Windows.h>
#include "../windowsMain.h"

Update::Update()
{
	{
		std::ifstream infile("version.txt");
		infile >> m_Version;
		infile.close();

		m_LatestVersion = m_Version;
	}

	{
		Web::DownLoadStatus status = Web::Nothing;
		Web::DownLoadFileFromGoogleDrive("11FT3jadKrJbzUEwpd5H2qUZF7bggyNUM", "ALoNOgkrtxj6M_1Z5JK-Qdpb7cg4:1747596794374", status);

		std::ifstream infile("LatestVersion.txt");
		infile >> m_LatestVersion;
		infile.close();

		if (m_LatestVersion.empty())
			m_LatestVersion = '.';

		if (m_LatestVersion != m_Version && m_LatestVersion[0] != '.')
			m_UpdateAvailable = true;
	}
}

void Update::OnImGuiRender()
{
	UI_DrawUpdateModal();
	UI_DrawStartUpdatingModal();
}

void Update::ShowUpdateModal()
{
	m_UpdateModalOpen = true;
}

void Update::ShowStartUpdatingModal()
{
	m_StartUpdatingModalOpen = true;
}

std::string Update::GetVersion() const
{
	return m_Version;
}

std::string Update::GetLatestVersion() const
{
	return m_LatestVersion;
}

bool Update::IsUpdateAvailable() const
{
	return m_UpdateAvailable;
}

void Update::StartOperationForUpdate()
{
	if (m_Updater || !m_UpdateAvailable)
	{
		return;
	}

	m_Updater = new std::thread(
		[this]()
		{
			Web::DownLoadStatus status = Web::Nothing;
			auto name = Web::DownLoadFileFromGoogleDrive("1aBeo7DlwrKIB9tZD7FSKWKxEdkyEHW88", "ALoNOglt_2hYbSiwowwywADKOhaq:1747598185917", status);

			if (status == Web::Finished)
			{
				std::string command = "powershell -command \"Expand-Archive -Path '";
				command += name;
				command += "' -DestinationPath '";
				command += (std::filesystem::current_path() / "toUpdate").u8string();
				command += "'\"";

				STARTUPINFO si = { sizeof(STARTUPINFO) };
				PROCESS_INFORMATION pi;
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;  // This hides the window

				// Create the process
				if (!CreateProcess(
					NULL,           // cmd executable
					(wchar_t*)std::wstring(command.begin(), command.end()).c_str(), // Command line (including arguments)
					NULL,                        // Process handle not inheritable
					NULL,                        // Thread handle not inheritable
					FALSE,                       // Set handle inheritance to FALSE
					CREATE_NO_WINDOW,            // Do not create a console window
					NULL,                        // Use parent's environment block
					NULL,                        // Use parent's starting directory 
					&si,                         // Pointer to STARTUPINFO structure
					&pi)                         // Pointer to PROCESS_INFORMATION structure
					)
				{
					std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
					m_UpdateFailed = false;

					m_ThreadEnded = true;
					return;
				}

				// Wait until the process completes
				WaitForSingleObject(pi.hProcess, INFINITE);

				// Close process and thread handles
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				std::cout << "Command executed successfully." << std::endl;


				//std::error_code ec;
				//std::filesystem::remove_all(name, ec);
				//if (ec)
				//{
				//	std::ofstream ef("ErrorFile.txt");
				//	ef << "func(std::filesystem::remove_all) " << ec << "path: " << name;
				//	ef.close();
				//}

				std::filesystem::copy_file(std::filesystem::current_path() / "Start.exe", std::filesystem::current_path() / "toUpdate" / "Update.exe", std::filesystem::copy_options::overwrite_existing);

				Process startProcess(L"toUpdate/Update.exe", L"");
			}
			else
				m_UpdateFailed = true;

			m_ThreadEnded = true;
		});

}

void Update::UI_DrawUpdateModal()
{
	if (!m_UpdateModalOpen)
		return;

	ImGui::OpenPopup("Update");
	m_UpdateModalOpen = ImGui::BeginPopupModal("Update", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_UpdateModalOpen)
	{
		auto image = Walnut::Application::Get().GetApplicationIcon();
		ImGui::Image((ImTextureID)image->GetRendererID(), { 48, 48 });
		ImGui::SameLine();
		Walnut::UI::ShiftCursorX(20.0f);
		ImGui::BeginGroup();
		ImGui::Text("LightSource is a Chess GUI");
		ImGui::Text("by C.Betsakos");
		ImGui::Text("Version: %s", m_Version.c_str());
		ImGui::Text("Latest Version: %s", m_LatestVersion.c_str());
		ImGui::Text("If you choose to update then make sure to save any opened files!");
		ImGui::EndGroup();

		if (Walnut::UI::ButtonCentered("Update"))
		{
			m_UpdateModalOpen = false;
			ImGui::CloseCurrentPopup();

			ShowStartUpdatingModal();
		}
		if (Walnut::UI::ButtonCentered("Close"))
		{
			m_UpdateModalOpen = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Update::UI_DrawStartUpdatingModal()
{
	if (!m_StartUpdatingModalOpen)
		return;
	ImGui::OpenPopup("Updating");
	m_StartUpdatingModalOpen = ImGui::BeginPopupModal("Updating", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_StartUpdatingModalOpen)
	{
		ImGui::Text("Updating LightSource to the latest version...");

		StartOperationForUpdate();
		
		if (m_ThreadEnded)
		{
			if (m_Updater->joinable())
				m_Updater->join();

			delete m_Updater;
			m_Updater = nullptr;
			m_ThreadEnded = false;

			if (m_UpdateFailed)
			{
				m_UpdateFailed = false;
				ImGui::Text("Update Failed!");
				ImGui::Text("Please try again later!");
				ImGui::NewLine();
				if (Walnut::UI::ButtonCentered("Close"))
				{
					m_StartUpdatingModalOpen = false;
					ImGui::CloseCurrentPopup();
				}
			}
			else
				Walnut::Application::Get().Close();
		}

		ImGui::EndPopup();
	}
}
