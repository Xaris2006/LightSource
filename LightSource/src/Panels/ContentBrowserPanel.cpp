#include "ContentBrowserPanel.h"

#include "Walnut/Application.h"

#include "../AppManagerChild.h"

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include <fstream>

extern bool g_AlreadyOpenedModalOpen;

namespace Panels {

	//extern bool p_SavedFile;

	static std::filesystem::path s_oldpath;
	static std::string s_inputNName;
	static bool s_openPopup = false;

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory("MyDocuments"), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/FileIcon.png");
		m_FileIconPGN = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/FileIconPGN.png");
		m_FileIconCOB = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/FileIconCOB.png");
	}
   
	void ContentBrowserPanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Content Browser", &m_viewPanel);
		
		if (ImGui::IsWindowHovered() && !ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup("Browser Popup");
		if (ImGui::BeginPopup("Browser Popup"))
		{
			if (ImGui::Selectable("New File"))
			{
				s_openPopup = true;
				s_inputNName = "NewFile.pgn";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("New Directory"))
			{
				s_openPopup = true;
				s_inputNName = "DirectoryName";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			//if (ImGui::Selectable("New Directory"))
			//{
			//	s_openPopup = true;
			//	s_inputNName = "";
			//	s_oldpath = m_CurrentDirectory;
			//	ImGui::CloseCurrentPopup();
			//}

			ImGui::EndPopup();
		}

		if (s_openPopup)
		{
			s_openPopup = false;
			ImGui::OpenPopup("New File/Directory");
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New File/Directory", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			if (ImGui::Button("Create"))
			{
				std::filesystem::path nPath = std::filesystem::path() / s_oldpath / s_inputNName;
				if (!nPath.extension().empty())
				{
					std::ofstream filepath(nPath.string());
					filepath.close();
				}
				else
					std::filesystem::create_directory(nPath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			if (ImGui::Button("Cansel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();

			ImGui::PushID(filenameString.c_str());
			auto icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			if (directoryEntry.path().extension().string() == ".pgn")
				icon = m_FileIconPGN;
			if (directoryEntry.path().extension().string() == ".cob")
				icon = m_FileIconCOB;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize });

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				//Bug when open file it does not reset the other panels
				if (path.extension().string() != ".pgn")
				{
					printf("Could not load %c - not a chess file", path.filename().string().c_str());
				}
				else
				{
					bool anwser = AppManagerChild::IsChessFileAvail(path);

					if (anwser)
					{
						ChessAPI::OpenChessFile(path.string());
						AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
					}
					else
					{
						g_AlreadyOpenedModalOpen = true;
					}
				}
			}
			

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && directoryEntry.is_regular_file())
				ImGui::OpenPopup("File Popup");

			if (ImGui::BeginPopup("File Popup"))
			{
				if (ImGui::Selectable("Open"))
				{
					//Bug when open file it does not reset the other panels
					if (path.extension().string() != ".pgn")
					{
						printf("Could not load %s - not a chess file", path.filename().string());
					}
					else
					{
						bool anwser = AppManagerChild::IsChessFileAvail(path);

						if (anwser)
						{
							ChessAPI::OpenChessFile(path.string());
							AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
						}
						else
						{
							g_AlreadyOpenedModalOpen = true;
						}
					}

					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Open in window"))
				{
					//Bug when open file it does not reset the other panels
					if (path.extension().string() != ".pgn")
					{
						printf("Could not load %s - not a chess file", path.filename().string());
					}
					else
					{
						bool anwser = AppManagerChild::IsChessFileAvail(path);

						if (anwser)
						{
							AppManagerChild::OpenChessFile(path);
						}
						else
						{
							g_AlreadyOpenedModalOpen = true;
						}
					}

					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Delete"))
				{
					std::filesystem::remove(path);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Rename"))
				{
					s_openPopup = true;
					s_inputNName = path.filename().string();
					s_oldpath = path;
					ImGui::CloseCurrentPopup();
				}


				ImGui::EndPopup();
			}

			if (s_openPopup)
			{
				s_openPopup = false;
				ImGui::OpenPopup("Rename Popup");
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Rename Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("Name", &s_inputNName);

				ImGui::NewLine();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
				if (ImGui::Button("Rename"))
				{
					std::string fileNpath = s_oldpath.string().substr(0, s_oldpath.string().size() - s_oldpath.filename().string().size() - 1) + '\\' + s_inputNName;
					std::filesystem::rename(s_oldpath, fileNpath);
					ImGui::CloseCurrentPopup();
				}
				ImGui::PopStyleColor();
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
				if (ImGui::Button("Cansel"))
					ImGui::CloseCurrentPopup();
				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();

			}

			int extensionIndex = filenameString.find('.');
			if (extensionIndex != std::string::npos)
				filenameString.erase(extensionIndex);

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		// TODO: status bar
		ImGui::End();
		
	}

	bool& ContentBrowserPanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}
