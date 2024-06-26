#include "ContentBrowserPanel.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include <fstream>

#include <atlstr.h>
#include <shlobj.h>

namespace Panels {

	static std::filesystem::path s_oldpath;
	static std::string s_inputNName;
	static ImVec4 s_textColor;

	static std::filesystem::path s_path;
	static bool s_openFilePopup = false;
	static bool s_openEmptyPopup = false;
	static bool s_openRenamePopup = false;
	static bool s_openNewPopup = false;

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(std::filesystem::current_path() / "chess_working_directory"), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/FileIconPGN.png");
		m_FileIconPGN = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/FileIconPGN.png");
		m_FileIconCOB = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/FileIconCOB.png");
		m_BackArrow = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/previous.png");

		s_textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
	}
   
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		static ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable;

		if (ImGui::BeginTable("table", 2, flags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));
			ImGui::Separator();
			ImGui::Text("Browser");
			ImGui::Separator();
			ImGui::PopFont();

			ImGui::BeginChild("tree", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y - 8 * ImGui::GetStyle().FramePadding.y));

			TreeDirectory(m_BaseDirectory);

			ImGui::Separator();

			PWSTR userFolderPath;
			HRESULT result = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &userFolderPath);

			if (result == S_OK)
			{
				TreeDirectory(std::filesystem::path(userFolderPath));
				ImGui::Separator();
			}

			CoTaskMemFree(static_cast<LPVOID>(userFolderPath));


			ImGui::EndChild();

			ImGui::TableSetColumnIndex(1);

			ImGui::Separator();

			std::filesystem::path semiPath = m_CurrentDirectory;
			std::vector<std::string> DirectoryNames;

			int indexEnd = 0;
			while (semiPath.has_parent_path() && semiPath.has_filename())
			{
				DirectoryNames.push_back(semiPath.filename().string());
				semiPath = semiPath.parent_path();
				indexEnd++;
			}


			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton((ImTextureID)m_BackArrow->GetRendererID(), { ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y, ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y })
				&& m_CurrentDirectory.has_filename())
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
			ImGui::SameLine(0, ImGui::CalcTextSize(" <-  ").x);
			ImGui::PopStyleColor();

			if (ImGui::Button(semiPath.root_name().string().c_str()))
			{
				m_CurrentDirectory = m_CurrentDirectory.root_path();
				goto DirectoryChange;
			}
			ImGui::SameLine();
			ImGui::Text("\\");

			for (int i = indexEnd - 1; i > -1; i--)
			{
				ImGui::SameLine();
				if (ImGui::Button(DirectoryNames[i].c_str()))
				{
					for (int j = 0; j < i; j++)
					{
						m_CurrentDirectory = m_CurrentDirectory.parent_path();
					}
					goto DirectoryChange;
				}
				ImGui::SameLine();
				ImGui::Text("\\");
			}

			ImGui::Separator();

			ImGui::BeginChild("Files", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y - 8 * ImGui::GetStyle().FramePadding.y));

			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				s_openEmptyPopup = true;
			
			

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
						printf("Could not load {0} - not a chess file", path.filename().string());
					}
					else
					{
						ChessAPI::SetNewChessGame(path.string());
					}
				}


				if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && directoryEntry.is_regular_file())
				{
					s_openFilePopup = true;
					s_path = path;
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

				ImGuiStyle& style = ImGui::GetStyle();

				float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + style.FramePadding.x * 2.0f;
				float avail = ImGui::GetContentRegionAvail().x;

				float off = (avail - actualSize) * 0.5f;
				if (off > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

				ImGui::TextWrapped(filenameString.c_str());

				ImGui::NextColumn();

				ImGui::PopID();
			}
			ImGui::Columns(1);

			ImGui::EndChild();

			ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 6 * ImGui::GetStyle().ItemSpacing.y - 6 * ImGui::GetStyle().FramePadding.y);

			ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 128, 256);
			ImGui::SliderFloat("Padding", &padding, 0, 32);

		DirectoryChange:
			ImGui::EndTable();
		}

		if (s_openFilePopup)
		{
			s_openFilePopup = false;

			ImGui::OpenPopup("File Popup");
		}

		FilePopup();

		if (s_openRenamePopup)
		{
			s_openRenamePopup = false;
			ImGui::OpenPopup("Rename Popup");
		}

		RenamePopup();
		
		if (s_openEmptyPopup)
		{
			s_openEmptyPopup = false;
			ImGui::OpenPopup("Empty Popup");
		}

		EmptyPopup();

		if (s_openNewPopup)
		{
			s_openNewPopup = false;
			ImGui::OpenPopup("New File/Directory");
		}

		NewPopup();

		ImGui::End();
	}

	void ContentBrowserPanel::TreeDirectory(const std::filesystem::path& directory)
	{
		bool openTree = false;

		ImVec4 vcolor = { 0, 0.66, 0.95, 1 };
		if (directory == m_CurrentDirectory)
			vcolor = { 0.38, 0.67, 0, 1 };

		ImGui::PushStyleColor(ImGuiCol_Text, vcolor);
		ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));
		if (ImGui::TreeNodeEx(directory.filename().string().c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
			openTree = true;
		ImGui::PopFont();
		ImGui::PopStyleColor();
		

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
		{
			m_CurrentDirectory = directory;
		}

		if(openTree)
		{
			for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
			{
				if (directoryEntry.is_directory())
				{
					TreeDirectory(directoryEntry);
				}
				else if(directoryEntry.is_regular_file())
				{
					ImGui::TreeNodeEx(directoryEntry.path().filename().string().c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (directoryEntry.path().extension().string() != ".pgn")
						{
							printf("Could not load {0} - not a chess file", directoryEntry.path().filename().string());
						}
						else
						{
							ChessAPI::SetNewChessGame(directoryEntry.path().string());
						}
					}
					
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						s_openFilePopup = true;
						s_path = directoryEntry.path();
					}
				}
			}
			ImGui::TreePop();
		}		
	}

	void ContentBrowserPanel::FilePopup()
	{
		if (ImGui::BeginPopup("File Popup"))
		{
			if (ImGui::Selectable("Open"))
			{
				//Bug when open file it does not reset the other panels
				if (s_path.extension().string() != ".pgn")
				{
					printf("Could not load {0} - not a chess file", s_path.filename().string());
				}
				else
				{
					ChessAPI::SetNewChessGame(s_path.string());
				}

				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Delete"))
			{
				std::filesystem::remove(s_path);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Rename"))
			{
				s_openRenamePopup = true;
				s_inputNName = s_path.filename().string();
				s_oldpath = s_path;
				ImGui::CloseCurrentPopup();
			}


			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::EmptyPopup()
	{
		if (ImGui::BeginPopup("Empty Popup"))
		{
			if (ImGui::Selectable("New File"))
			{
				s_openNewPopup = true;
				s_inputNName = "NewFile.pgn";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("New Directory"))
			{
				s_openNewPopup = true;
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
	}

	void ContentBrowserPanel::RenamePopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Rename Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			if (ImGui::Button("Rename"))
			{
				std::string fileNpath = s_oldpath.string().substr(0, s_oldpath.string().size() - s_oldpath.filename().string().size() - 1) + '\\' + s_inputNName;
				std::filesystem::rename(s_oldpath, fileNpath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Cansel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::NewPopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New File/Directory", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
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
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Cansel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}
	}

}
