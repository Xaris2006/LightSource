#include "ContentBrowserPanel.h"

#include "../Creator.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include <atlstr.h>
#include <shlobj.h>

namespace Panels {

	static ImVec4 s_textColor;

	static std::filesystem::path s_path;
	static bool s_openFilePopup = false;

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(MYDOCUMENTS), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIconPGN = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/FileIconPGN.png");
		m_BackArrow = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/previous.png");

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

			Walnut::UI::TextCentered("Browser");

			ImGui::Separator();
			ImGui::Separator();
			ImGui::PopFont();

			ImGui::BeginChild("tree", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

			TreeDirectory(m_BaseDirectory);

			ImGui::Separator();

			std::filesystem::path userPath;

			//windows only
			{
				PWSTR userFolderPath;
				HRESULT result = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &userFolderPath);

				if (result == S_OK)
				{
					userPath = std::filesystem::path(userFolderPath);
				}
				CoTaskMemFree(static_cast<LPVOID>(userFolderPath));
			}

			TreeDirectory(userPath / "Documents");
			ImGui::Separator();
			
			TreeDirectory(userPath / "Downloads");
			ImGui::Separator();

			TreeDirectory(userPath / "Desktop");
			ImGui::Separator();

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
			ImGui::Text(">");

			for (int i = indexEnd - 1; i > -1; i--)
			{
				ImGui::SameLine();
				ImGui::PushID(i);
				if (ImGui::Button(DirectoryNames[i].c_str()))
				{
					for (int j = 0; j < i; j++)
					{
						m_CurrentDirectory = m_CurrentDirectory.parent_path();
					}
					ImGui::PopID();
					goto DirectoryChange;
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::Text(">");
			}

			ImGui::Separator();

			ImGui::BeginChild("Files", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

			static float padding = 32.0f;
			static float thumbnailSize = 128.0f;
			static float cellSize;
			cellSize = thumbnailSize + padding;

			static float panelWidth;
			panelWidth = ImGui::GetContentRegionAvail().x;
			static int columnCount;
			columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;

			ImGui::PushStyleColor(ImGuiCol_Text, { 0.38, 0.67, 0, 1 });
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			static float oldCursorY = 0;
			oldCursorY = ImGui::GetCursorPosY();
			ImGui::SetCursorPosY(oldCursorY + 5);

			ImGui::Text("Search");

			ImGui::PopFont();
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::SetCursorPosY(oldCursorY);

			static ImGuiTextFilter filter;
			filter.Draw("##SearchFilter", ImGui::GetContentRegionAvail().x / 3);

			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Columns(columnCount, 0, false);

			for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				std::u8string filenameU8String = path.filename().u8string();
				std::string filenameString = std::string(filenameU8String.begin(), filenameU8String.end());
				
				if (directoryEntry.is_regular_file() && !path.has_extension())
					continue;

				if (path.has_extension() && path.extension().string() != ".pgn")
					continue;

				if (!filter.PassFilter(filenameString.c_str()))
					continue;

				ImGui::PushID(filenameString.c_str());
				auto icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIconPGN;
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize });
				ImGui::PopStyleColor();

				if (directoryEntry.is_regular_file())
				{
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						s_openFilePopup = true;
						s_path = path;
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						Tools::COBCreator::Creator::Get().AddFile(path);

					if (ImGui::BeginDragDropSource())
					{
						std::filesystem::path relativePath(path);
						const wchar_t* itemPath = relativePath.c_str();
						ImGui::SetDragDropPayload("ADD_FILE", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
						ImGui::EndDragDropSource();
					}


				}

				if (directoryEntry.is_directory())
				{
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						m_CurrentDirectory /= path.filename();
				}
				ImGuiStyle& style = ImGui::GetStyle();

				int extensionIndex = filenameString.find_last_of('.');
				if (extensionIndex != std::string::npos)
					filenameString.erase(extensionIndex);

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

			ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 6 * ImGui::GetStyle().ItemSpacing.y);

			ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 128, 256, "%.0f");
			//ImGui::SliderFloat("Padding", &padding, 0, 32);

		DirectoryChange:
			ImGui::EndTable();
		}

		if (s_openFilePopup)
		{
			s_openFilePopup = false;

			ImGui::OpenPopup("File Popup");
		}

		FilePopup();

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

		if (openTree)
		{
			for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
			{
				if (directoryEntry.is_directory())
				{
					TreeDirectory(directoryEntry);
				}
				else if (directoryEntry.is_regular_file())
				{
					if (directoryEntry.path().extension().string() != ".pgn")
						continue;

					ImGui::TreeNodeEx(directoryEntry.path().filename().string().c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					
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
			if (ImGui::Selectable("Add File"))
			{
				Tools::COBCreator::Creator::Get().AddFile(s_path);

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Open Explorer"))
			{
				std::string cmd = "explorer " + s_path.parent_path().string();
				std::system(cmd.c_str());
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}
