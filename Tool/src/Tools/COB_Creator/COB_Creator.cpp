#include "COB_Creator.h"

#include "Creator.h"

#include "misc/cpp/imgui_stdlib.h"
#include "Walnut/UI/UI.h"

#include "../../ChessLab-Lobby/src/Windows/WindowsUtils.h"
#include "../../ChessLab-Lobby/src/windowsMain.h"

#include "GLFW/glfw3.h"

#include <fstream>
#include <iostream>

extern Walnut::ApplicationSpecification g_spec;
extern std::string g_AppDirectory;

static std::vector<std::string> s_arg;

namespace Tools::COBCreator
{
	void Layer::OnAttach()
	{
		Chess::PgnManager::Init();
		Creator::Init();
		
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TableBorderLight] = ImColor(255, 225, 135, 80);

		m_PlayIcon = std::make_shared<Walnut::Image>("Resources\\Icons\\PlayButton.png");
		m_StopIcon = std::make_shared<Walnut::Image>("Resources\\Icons\\StopButton.png");
		m_ResumeIcon = std::make_shared<Walnut::Image>("Resources\\Icons\\ResumeButton.png");
		m_PauseIcon = std::make_shared<Walnut::Image>("Resources\\Icons\\PauseButton.png");

		m_ContentBrowserPanel = std::make_unique<Panels::ContentBrowserPanel>();

		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());
		glfwFocusWindow(Walnut::Application::Get().GetWindowHandle());

		Creator::Get().SetName("MyCOBname");
	}

	void  Layer::OnDetach()
	{
		Creator::ShutDown();
	}

	void Layer::OnUIRender()
	{
		Creator::OnUpdate();

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			ImGui::BeginDisabled(Creator::Get().GetStatus() != Creator::Nothing);

			if (ImGui::IsKeyPressed(ImGuiKey_A))
			{
				AddFile();
			}

			ImGui::BeginDisabled(Creator::Get().GetSize() == 0);

			if (ImGui::IsKeyPressed(ImGuiKey_B))
			{
				Creator::Get().StartBuild();
			}

			ImGui::EndDisabled();
			ImGui::EndDisabled();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F4) && (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)))
		{
			Walnut::Application::Get().Close();
		}

		ImGui::Begin("Build Area");

		auto files = Creator::Get().GetFiles();

		ImGui::BeginDisabled(Creator::Get().GetStatus() != Creator::Nothing);

		static int fileToRemove = 0;

		if (ImGui::BeginTable("Files", 5, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg,
			ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * 2 * ImGui::GetStyle().ItemSpacing.y)))
		{
			ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_NoHide);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupColumn("Full Path", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupColumn("Games", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);

			ImGui::TableHeadersRow();

			for (int i = 0; i < files.size(); i++)
			{
				ImGui::TableNextRow();

				for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				{
					if (!ImGui::TableSetColumnIndex(column) && column > 0)
						continue;
					
					ImGui::PushID(i * ImGui::TableGetColumnCount() + column);
					
					if (column == 0)
						ImGui::Text("%d", i + 1);
					else if(column == 1)
					{
						ImGui::Text(files[i].filename().string().c_str());
					}
					else if(column == 2)
					{
						ImGui::Text(files[i].string().c_str());
					}
					else if(column == 3)
					{
						ImGui::Text("%d", Creator::Get().GetFileSize(i));
					}
					else
					{
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.7f, 0.1f, 0.1f, 0.65f)));
						ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

						ImGui::SetNextItemOpen(true);

						ImGui::Selectable("Remove");

						ImGui::PopStyleColor(2);

						if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						{
							fileToRemove = i + 1;
						}
					}
					
					ImGui::PopID();
				}
			}

			ImGui::EndTable();
		}

		if (fileToRemove)
		{
			Creator::Get().RemoveFile(fileToRemove - 1);
			fileToRemove = 0;
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ADD_FILE"))
			{
				const wchar_t* wpath = (const wchar_t*)payload->Data;
				std::filesystem::path spath(wpath);
		
				Creator::Get().AddFile(spath);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();


		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.58f, 0.97f, 1.0f));

		ImGui::Text(("Creation Path: " + (MYDOCUMENTS / "Books").string() + '\\').c_str());

		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);

		ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517192123252729313335373941").x);

		ImGui::InputText("##COBName", &Creator::Get().GetName());

		ImGui::EndDisabled();

		ImGui::Separator();

		if (Creator::Get().GetStatus() == Creator::Nothing)
		{
			ImGui::BeginDisabled(files.empty());

			if (ImGui::ImageButton((ImTextureID)m_PlayIcon->GetRendererID(), { 22, 22 }))
			{
				Creator::Get().Creator::StartBuild();
			}

			ImGui::EndDisabled();
		}
		else
		{
			if (Creator::Get().GetStatus() == Creator::Paused)
			{
				if (ImGui::ImageButton((ImTextureID)m_ResumeIcon->GetRendererID(), { 22, 22 }))
				{
					Creator::Get().Creator::ResumeBuild();
				}
			}
			else
			{
				if (ImGui::ImageButton((ImTextureID)m_PauseIcon->GetRendererID(), { 22, 22 }))
				{
					Creator::Get().Creator::PauseBuild();
				}
			}

		}

		ImGui::BeginDisabled(Creator::Get().GetStatus() == Creator::Nothing);

		ImGui::SameLine();

		if (ImGui::ImageButton((ImTextureID)m_StopIcon->GetRendererID(), { 22, 22 }))
		{
			Creator::Get().Creator::EndBuild();
		}

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,		  ImVec4(0.3f, 0.58f, 0.97f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.3f, 0.58f, 0.97f, 0.6f));

		ImGui::Button((std::to_string((int)Creator::Get().GetPercentage()) + '%').c_str(), { 55, ImGui::GetFrameHeight()});
		
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		
		auto ycursor = ImGui::GetCursorPosY();
		auto xcursor = ImGui::GetCursorPosX();
		
		float availx = ImGui::GetContentRegionAvail().x;
		ImGui::Button("##end", ImVec2(availx, 0));

		ImGui::SetCursorPosY(ycursor);
		ImGui::SetCursorPosX(xcursor);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));

		ImGui::Button("##bar", ImVec2(availx * Creator::Get().GetPercentage() / 100, 0));

		ImGui::PopStyleColor(3);
		
		ImGui::EndDisabled();

		ImGui::End();

		ImGui::BeginDisabled(Creator::Get().GetStatus() != Creator::Nothing);

		m_ContentBrowserPanel->OnImGuiRender();

		ImGui::EndDisabled();

		UI_DrawAboutModal();
	}

	void Layer::ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	void Layer::AddFile()
	{
		std::string filepath = Windows::Utils::OpenFile("Chess File (*.pgn)\0*.pgn\0");

		if (!filepath.empty())
		{
			Creator::Get().AddFile(filepath);
		}
	}

	void Layer::UI_DrawAboutModal()
	{
		if (!m_AboutModalOpen)
			return;

		ImGui::OpenPopup("About");
		m_AboutModalOpen = ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (m_AboutModalOpen)
		{
			auto image = Walnut::Application::Get().GetApplicationIcon();
			ImGui::Image((ImTextureID)image->GetRendererID(), { 48, 48 });

			ImGui::SameLine();
			Walnut::UI::ShiftCursorX(20.0f);

			ImGui::BeginGroup();
			ImGui::Text("COB creator is a Chess Opening Book creator Tool");
			ImGui::Text("by C.Betsakos");
			ImGui::EndGroup();

			if (Walnut::UI::ButtonCentered("Close"))
			{
				m_AboutModalOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	Walnut::Application* CreateApplication(int argc, char** argv)
	{
		g_spec.Name = "Chess Opening Book Creator";
		g_spec.CustomTitlebar = true;
		g_spec.AppIconPath = "ToolIconCOB.png";
		g_spec.IconPath = "ToolIconCOB.png";
		g_spec.HoveredIconPath = "ToolIconCOB.png";
		g_spec.FuncIconPressed = []()
			{

			};

		//fix arg
		s_arg.emplace_back(argv[0]);
		for (int i = 1; i < argc; i++)
		{
			if (std::filesystem::path(s_arg[s_arg.size() - 1]).has_extension())
				s_arg.emplace_back(argv[i]);
			else
			{
				s_arg[s_arg.size() - 1] += ' ';
				s_arg[s_arg.size() - 1] += argv[i];
			}
		}

		g_AppDirectory = std::filesystem::path(s_arg[0]).parent_path().string();

#if defined(WL_DIST)
		std::filesystem::current_path(g_AppDirectory);
#endif

		Walnut::Application* app = new Walnut::Application(g_spec, 117 - 15);

		//app->SetMinImGuiWindowSize(370.0f);
		app->SetDockNodeFlags(ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

		std::shared_ptr<Layer> COBCLayer = std::make_shared<Layer>();
		app->PushLayer(COBCLayer);
		app->SetMenubarCallback([app, COBCLayer]()
			{
				if (ImGui::BeginMenu("Menu"))
				{
					ImGui::BeginDisabled(Creator::Get().GetStatus() != Creator::Nothing);

					if (ImGui::MenuItem("Add File", "Ctr+A"))
					{
						COBCLayer->AddFile();
					}

					ImGui::BeginDisabled(Creator::Get().GetSize() == 0);

					if (ImGui::MenuItem("Build", "Ctr+B"))
					{
						Creator::Get().StartBuild();
					}
					
					ImGui::EndDisabled();
					ImGui::EndDisabled();

					if (ImGui::MenuItem("Exit", "Alt+F4"))
					{
						Walnut::Application::Get().Close();
					}

					ImGui::EndMenu();

				}

				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("About"))
					{
						COBCLayer->ShowAboutModal();
					}
					ImGui::EndMenu();
				}
			});

		return app;
	}
}