#include "ToolsPanel.h"

#include <fstream>

#include "imgui.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"

#include "../windowsMain.h"
#include "../Manager/ToolManager.h"

namespace Panels
{
	ToolsPanel::ToolsPanel()
	{
		FindAvailableTools();
		FindDownloadableTools();
	}

	void ToolsPanel::OnImGuiRender()
	{
		ImGui::Begin("Tools");

		if (ImGui::BeginTable("table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginChild("##View", ImVec2(0, ImGui::GetContentRegionAvail().y - 10));

			ImGui::BeginTabBar("ProfileMenu");

			if (ImGui::BeginTabItem("Available"))
			{
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


				ImGui::Columns(columnCount, 0, false);

				for (int i = 0; i < m_AvailableTools.size(); i++)
				{
					auto& path = m_AvailableTools[i];
					std::string filenameString = path.filename().string();

					ImGui::PushID(filenameString.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					if (ImGui::ImageButton((ImTextureID)m_ToolIcons[i]->GetRendererID(), { thumbnailSize, thumbnailSize }))
						m_TargetedToolIndex = i;

					ImGui::PopStyleColor();

					float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
					float avail = ImGui::GetContentRegionAvail().x;

					float off = (avail - actualSize) * 0.5f;
					if (off > 0.0f)
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

					ImGui::TextWrapped(filenameString.c_str());

					ImGui::NextColumn();

					ImGui::PopID();
				}
				ImGui::Columns(1);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Download"))
			{


				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();

			ImGui::EndChild();

			ImGui::TableSetColumnIndex(1);

			bool unistallCurrentTool = false;

			if (m_TargetedToolIndex > -1)
			{
				float size = min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) * 0.8f;
				
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size) * 0.5f);
				ImGui::Image((ImTextureID)m_ToolIcons[m_TargetedToolIndex]->GetRendererID(), ImVec2(size, size));
				
				std::string filenameString = m_AvailableTools[m_TargetedToolIndex].filename().string();
				
				{

					float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
					float avail = ImGui::GetContentRegionAvail().x;

					float off = (avail - actualSize) * 0.5f;
					if (off > 0.0f)
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

					ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

					ImGui::TextWrapped(filenameString.c_str());

					ImGui::PopFont();
				}

				ImGui::Separator();

				ImGui::NewLine();

				if(!Manager::ToolManager::Get()->IsToolRunning(filenameString))
				{

					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

						float actualSize = ImGui::CalcTextSize(" Open ").x + ImGui::CalcTextSize(" Unistall ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
						float avail = ImGui::GetContentRegionAvail().x;

						float off = (avail - actualSize) * 0.5f;
						if (off > 0.0f)
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);


						if (ImGui::Button("Open"))
						{
							Manager::ToolManager::Get()->RunTool(filenameString);
						}

						ImGui::PopStyleColor(3);
					}

					ImGui::SameLine();

					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

						if (ImGui::Button("Unistall"))
							unistallCurrentTool = true;

						ImGui::PopStyleColor(3);
					}
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

					if (Walnut::UI::ButtonCentered("Close"))
						Manager::ToolManager::Get()->ShutdownTool(filenameString);

					ImGui::PopStyleColor(3);
				}

				ImGui::NewLine();

				ImGui::Separator();

				ImGui::BeginChild("Description", ImVec2(0, ImGui::GetContentRegionAvail().y /2), true);

				Walnut::UI::TextCentered("Description");

				ImGui::NewLine();

				ImGui::TextWrapped(m_ToolLabelNameToValue[filenameString + "Description"].c_str());

				ImGui::EndChild();
				
				ImGui::BeginChild("Devaloper Details", ImVec2(0, ImGui::GetContentRegionAvail().y - 10), true);

				Walnut::UI::TextCentered("Devaloper Details");

				ImGui::NewLine();

				ImGui::TextWrapped(m_ToolLabelNameToValue[filenameString + "Devaloper_Details"].c_str());

				ImGui::EndChild();

			}
			else
			{
				ImVec2 textSize = ImGui::CalcTextSize("Select a Tool to interact with it!");

				{
					float actualSizeX = textSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
					float availX = ImGui::GetContentRegionAvail().x;

					float offX = (availX - actualSizeX) * 0.5f;
					if (offX > 0.0f)
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);
				}

				{
					float actualSizeY = textSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
					float availY = ImGui::GetContentRegionAvail().y;

					float offY = (availY - actualSizeY) * 0.5f;
					if (offY > 0.0f)
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offY);
				}

				ImGui::Text("Select a Tool to interact with it!");
			}
			
			if (unistallCurrentTool)
			{
				std::string filenameString = m_AvailableTools[m_TargetedToolIndex].filename().string();

				std::filesystem::remove_all(m_AvailableTools[m_TargetedToolIndex]);

				m_AvailableTools.erase(m_AvailableTools.begin() + m_TargetedToolIndex);

				m_ToolLabelNameToValue.erase(filenameString + "Icon");
				m_ToolLabelNameToValue.erase(filenameString + "Description");
				m_ToolLabelNameToValue.erase(filenameString + "Devaloper_Details");

				m_ToolIcons.erase(m_ToolIcons.begin() + m_TargetedToolIndex);

				std::ofstream tools(std::filesystem::current_path() / "LightSourceApp\\MyDocuments\\Tools\\Tools.txt");
				for(int i = 0; i < m_AvailableTools.size() - 1; i++)
					tools << m_AvailableTools[i].filename().string() << '\n';

				tools << m_AvailableTools[m_AvailableTools.size() - 1].filename().string();

				m_TargetedToolIndex = -1;
				unistallCurrentTool = false;
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}

	void ToolsPanel::FindAvailableTools()
	{
		m_AvailableTools.clear();
		m_ToolLabelNameToValue.clear();
		m_ToolIcons.clear();
		m_TargetedToolIndex = -1;

		std::ifstream pathFile(std::filesystem::current_path() / "LightSourceApp\\MyDocuments\\Tools\\Tools.txt");
		while (pathFile.good())
		{
			std::string path;
			pathFile >> path;
			m_AvailableTools.emplace_back(std::filesystem::current_path() / "LightSourceApp\\MyDocuments\\Tools" / path);
		}

		for (auto& path : m_AvailableTools)
		{
			std::string filenameString = path.filename().string();

			std::ifstream toolDetails(path / "tool.details", std::ios::binary);
			toolDetails.seekg(0, std::ios_base::end);
			std::streampos maxSize = toolDetails.tellg();
			toolDetails.seekg(0, std::ios_base::beg);

			if (maxSize <= 0)
				continue;

			char* data = new char[maxSize];
			toolDetails.read(data, maxSize);
			toolDetails.close();

			bool labelName = true;
			std::string name = "", value = "";

			for (size_t i = 0; i < maxSize; i++)
			{
				if (data[i] == '\n' || data[i] == '\r')
					continue;
				if (labelName && data[i] == ' ')
					continue;

				if (data[i] == '[')
				{
					labelName = false;
					continue;
				}

				if (data[i] == ']')
				{
					labelName = true;

					m_ToolLabelNameToValue[filenameString + name] = value;

					name.clear();
					value.clear();

					continue;
				}

				if (labelName)
					name += data[i];
				else
					value += data[i];

			}

			delete[] data;

			m_ToolIcons.push_back(std::make_shared<Walnut::Image>((path / m_ToolLabelNameToValue[filenameString + "Icon"]).string()));
			Manager::ToolManager::Get()->AddTool(path / (filenameString + ".exe"), filenameString);
		}
	}

	void  ToolsPanel::FindDownloadableTools()
	{

	}


}