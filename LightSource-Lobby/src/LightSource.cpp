#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "Windows/WindowsUtils.h"
#include "AppManager.h"

#include "Panels/ContentBrowserPanel.h"

#include <iostream>
#include <array>

#include "GLFW/glfw3.h"


AppManager g_AppManager;

std::string g_AppDirectory;
Walnut::ApplicationSpecification g_spec;

class LobbyLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());

		if (__argc > 1)
		{
			if (chess::IsFileValidFormat(__argv[1], ".pgn"))
			{
				g_AppManager.CreateApp(__argv[1]);
			}
			//else if (chess::IsFileValidFormat(commandLineArgs[1], ".cob"))
			//{
			//	//m_ChessPanel.IsOpeningBookPanelOpen() = true;
			//	m_chess.openBook.OpenCOBfile(commandLineArgs[1]);
			//}
		}

		m_HomeIcon = std::make_shared<Walnut::Image>("LightSourceApp\\Resources\\menu\\home-button.png");
		m_ProfilIcon = std::make_shared<Walnut::Image>("LightSourceApp\\Resources\\menu\\user.png");
		m_WebIcon = std::make_shared<Walnut::Image>("LightSourceApp\\Resources\\menu\\internet.png");
		m_ToolsIcon = std::make_shared<Walnut::Image>("LightSourceApp\\Resources\\menu\\spanner.png");
		m_HelpIcon = std::make_shared<Walnut::Image>("LightSourceApp\\Resources\\menu\\question-mark.png");

		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_TableBorderLight] = ImColor(255, 225, 135, 80);

	}

	virtual void OnUIRender() override
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_N))
			{
				New();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_O))
			{
				Open();
			}
		}

		//ImGui::ShowDemoWindow();

		//menu
		ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoDecoration);

		ImVec2 IconSize = { ImGui::GetContentRegionAvail().x - 2*ImGui::GetStyle().ItemSpacing.x, ImGui::GetContentRegionAvail().x - 2* ImGui::GetStyle().ItemSpacing.x };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		//ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y/8);
		if (ImGui::ImageButton((ImTextureID)m_HomeIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 0;
		}
		if (ImGui::ImageButton((ImTextureID)m_ProfilIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 1;
		}
		if (ImGui::ImageButton((ImTextureID)m_WebIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 2;
		}
		if (ImGui::ImageButton((ImTextureID)m_ToolsIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 3;
		}
		if (ImGui::ImageButton((ImTextureID)m_HelpIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 4;
		}

		ImGui::PopStyleColor();

		ImGui::End();

		UI_DrawAboutModal();

		if (m_MenuIntex == 0)
		{
			m_ContentBrowserPanel.OnImGuiRender();
		}
		else if (m_MenuIntex == 1)
		{
			
		}
		else if (m_MenuIntex == 2)
		{

		}
		else if (m_MenuIntex == 3)
		{

		}
		else if (m_MenuIntex == 4)
		{

		}

		
	}

	void UI_DrawAboutModal()
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
			ImGui::Text("LightSource is a Chess GUI");
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

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	void New()
	{
		g_AppManager.CreateApp("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile("Chess Database (*.pgn)\0*.pgn\0");
		if (!filepath.empty())
			g_AppManager.CreateApp(filepath);
	}

private:

	Panels::ContentBrowserPanel m_ContentBrowserPanel;

	int m_MenuIntex = 0;

	//menu Icons
	std::shared_ptr<Walnut::Image> m_HomeIcon;
	std::shared_ptr<Walnut::Image> m_ProfilIcon;
	std::shared_ptr<Walnut::Image> m_WebIcon;
	std::shared_ptr<Walnut::Image> m_ToolsIcon;
	std::shared_ptr<Walnut::Image> m_HelpIcon;

	bool m_AboutModalOpen = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	g_spec.Name = "Light Source";
	g_spec.CustomTitlebar = true;
	g_spec.IconPath = "LightSourceApp\\Resources\\LightSource\\ls.png";
	g_spec.HoveredIconPath = "LightSourceApp\\Resources\\LightSource\\lsOn.png";
	g_spec.FuncIconPressed = []()
		{
			g_AppManager.CreateApp("");
		};
	g_AppDirectory = std::filesystem::path(argv[0]).parent_path().string();

#if defined(WL_DIST)
	std::filesystem::current_path(g_AppDirectory);
#endif

	Walnut::Application* app = new Walnut::Application(g_spec);
	
	//app->SetMinImGuiWindowSize(370.0f);
	app->SetDockNodeFlags(ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	std::shared_ptr<LobbyLayer> chessLayer = std::make_shared<LobbyLayer>();
	app->PushLayer(chessLayer);
	app->SetMenubarCallback([app, chessLayer]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctr+N"))
			{
				chessLayer->New();
			}
			if (ImGui::MenuItem("Open", "Ctr+O"))
			{
				chessLayer->Open();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				chessLayer->ShowAboutModal();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}