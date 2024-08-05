#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "Windows/WindowsUtils.h"

#include "AppManagerChild.h"

#include "ChessAPI.h"

#include "ImGuiBoard.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/DatabasePanel.h"
#include "Panels/GamePropertiesPanel.h"
#include "Panels/NotePanel.h" //will be included in Move panel
#include "Panels/MovePanel.h"
#include "Panels/OpeningBookPanel.h"
#include "Panels/EnginePanel.h"

#include <iostream>
#include <array>
#include <fstream>

#include "GLFW/glfw3.h"

std::string g_AppDirectory;
Walnut::ApplicationSpecification g_spec;
bool g_AlreadyOpenedModalOpen = false;

class ChessLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());

		AppManagerChild::Init();
		ChessAPI::Init();

		m_ChessBoard.OnAttach();

		if (__argc > 1)
		{
			std::string cmd = "";

			for (int i = 1; i < __argc - 1; i++)
			{
				cmd += __argv[i];
				cmd += ' ';
			}
			cmd += __argv[__argc - 1];

			if (chess::IsFileValidFormat(cmd, ".pgn"))
			{
				ChessAPI::OpenChessFile(cmd);
				AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
			}
			//else if (chess::IsFileValidFormat(commandLineArgs[1], ".cob"))
			//{
			//	//m_ChessPanel.IsOpeningBookPanelOpen() = true;
			//	m_chess.openBook.OpenCOBfile(commandLineArgs[1]);
			//}
		}

		std::ifstream lsIni("lightsource.ini");
		std::string name;
		lsIni >> name >> ContentBrowserPanelViewStatus();
		lsIni >> name >> GamePropertiesPanelViewStatus();
		lsIni >> name >> NotePanelViewStatus();
		lsIni >> name >> MovePanelViewStatus();
		lsIni >> name >> OpeningBookPanelViewStatus();
		lsIni.close();

		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	}

	virtual void OnDetach() override
	{
		AppManagerChild::ShutDown();
	}

	virtual void OnUIRender() override
	{	
		AppManagerChild::OnUpdate();

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

			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
					SaveAs();
				else
					Save();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_F))
			{
				FlipBoard();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_E))
			{
				OpenEditor();
			}
			
			if (ImGui::IsKeyPressed(ImGuiKey_B))
			{
				ImGui::SetClipboardText(ChessAPI::GetFEN().c_str());
			}

			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
			{
				OpenChessEngine();
			}
			
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			{
				CloseChessEngine();
			}
		}


		m_ChessBoard.OnUIRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_DatabasePanel.OnImGuiRender();
		m_NotePanel.OnImGuiRender();
		m_OpeningBookPanel.OnImGuiRender();
		m_GamePropertiesPanel.OnImGuiRender();
		m_MovePanel.OnImGuiRender();
		m_ChessEnginePanel.OnImGuiRender();
		
		UI_DrawAboutModal();
		AlreadyOpenedModal();
		//ImGui::ShowDemoWindow();
		//ImGui::ShowMetricsWindow();
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

	void AlreadyOpenedModal()
	{
		if (!g_AlreadyOpenedModalOpen)
			return;
		ImGui::OpenPopup("Error-File Is Already Opened");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		
		g_AlreadyOpenedModalOpen = ImGui::BeginPopupModal("Error-File Is Already Opened", 0, ImGuiWindowFlags_NoResize);
		
		if (g_AlreadyOpenedModalOpen)
		{
			ImGui::TextWrapped("The file that you are trying to open is already opened in a different LightSource Window!");

			ImGui::NewLine();

			ImGui::PushID("in2");

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Close").x - 18);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Close"))
			{
				g_AlreadyOpenedModalOpen = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();

			ImGui::PopID();

			ImGui::EndPopup();
		}
	}

	bool& ContentBrowserPanelViewStatus()
	{
		return m_ContentBrowserPanel.IsPanelOpen();
	}
	
	bool& GamePropertiesPanelViewStatus()
	{
		return m_GamePropertiesPanel.IsPanelOpen();
	}

	bool& NotePanelViewStatus()
	{
		return m_NotePanel.IsPanelOpen();
	}

	bool& MovePanelViewStatus()
	{
		return m_MovePanel.IsPanelOpen();
	}
	
	bool& OpeningBookPanelViewStatus()
	{
		return m_OpeningBookPanel.IsPanelOpen();
	}

	void OpenChessEngine(const std::string& path)
	{
		m_ChessEnginePanel.OpenChessEngine(path);
	}

	void OpenChessEngine()
	{
		OpenChessEngine(m_ChessEnginePanel.GetDefaultEngine());
	}

	void CloseChessEngine()
	{
		m_ChessEnginePanel.CloseChessEngine();
		m_ChessEnginePanel.Reset();
	}

	void OpenEditor()
	{
		m_ChessBoard.OpenEditor();
	}

	void FlipBoard()
	{
		m_ChessBoard.FlipBoard();
	}

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	void New()
	{
		ChessAPI::OpenChessFile("");

		AppManagerChild::OwnChessFile("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile("Chess Database (*.pgn)\0*.pgn\0");
		if (!filepath.empty())
		{
			bool anwser = AppManagerChild::IsChessFileAvail(filepath);

			if (anwser)
			{
				ChessAPI::OpenChessFile(filepath);
				AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
			}
			else
			{
				g_AlreadyOpenedModalOpen = true;
			}
		}
	}

	void SaveAs()
	{
		std::string filepath = Windows::Utils::SaveFile("Chess Database (*.pgn)\0*.pgn\0");
		if (!filepath.empty())
		{
			ChessAPI::OverWriteChessFile(filepath);
			ChessAPI::OpenChessFile(filepath);
			AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
		}
	}

	void Save()
	{
		if (ChessAPI::GetPgnFileName() == "New Game")
		{
			SaveAs();
		}
		else
			ChessAPI::OverWriteChessFile("");
	}


private:

	ImGuiBoard m_ChessBoard;
	Panels::ContentBrowserPanel m_ContentBrowserPanel;
	Panels::DatabasePanel m_DatabasePanel;
	Panels::GamePropertiesPanel m_GamePropertiesPanel;
	Panels::NotePanel m_NotePanel;
	Panels::MovePanel m_MovePanel;
	Panels::OpeningBookPanel m_OpeningBookPanel;
	Panels::EnginePanel m_ChessEnginePanel;

	bool m_AboutModalOpen = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	g_spec.Name = "Light Source";
	g_spec.CustomTitlebar = true;
	g_spec.AppIconPath = "Resources\\LightSource\\lsb.png";
	g_spec.IconPath = "Resources\\LightSource\\ls.png";
	g_spec.HoveredIconPath = "Resources\\LightSource\\lsOn.png";
	g_spec.FuncIconPressed = []()
		{
			AppManagerChild::OpenChessFile();
		};
	g_AppDirectory = std::filesystem::path(argv[0]).parent_path().string();

#if defined(WL_DIST)
	std::filesystem::current_path(g_AppDirectory);
#endif

	Walnut::Application* app = new Walnut::Application(g_spec, 237);
	std::shared_ptr<ChessLayer> chessLayer = std::make_shared<ChessLayer>();
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
			if (ImGui::MenuItem("Save", "Ctr+S"))
			{
				chessLayer->Save();
			}
			if (ImGui::MenuItem("Save As", "Ctr+Shift+S"))
			{
				chessLayer->SaveAs();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Content Browser", 0, &chessLayer->ContentBrowserPanelViewStatus())){}
			if (ImGui::MenuItem("Game Properties", 0, &chessLayer->GamePropertiesPanelViewStatus())){}
			if (ImGui::MenuItem("Notes",		   0, &chessLayer->NotePanelViewStatus())){}
			if (ImGui::MenuItem("Moves",		   0, &chessLayer->MovePanelViewStatus())){}
			if (ImGui::MenuItem("Opening Book",    0, &chessLayer->OpeningBookPanelViewStatus())){}

			ImGui::Separator();

			if (ImGui::MenuItem("Set current View Style as default"))
			{
				std::ofstream lsIni("lightsource.ini");
				lsIni << "Content_Browser" << ' ' << chessLayer->ContentBrowserPanelViewStatus() << '\n';
				lsIni << "Game_Properties" << ' ' << chessLayer->GamePropertiesPanelViewStatus() << '\n';
				lsIni << "Notes"		   << ' ' << chessLayer->NotePanelViewStatus() << '\n';
				lsIni << "Moves"		   << ' ' << chessLayer->MovePanelViewStatus() << '\n';
				lsIni << "Opening_Book"	   << ' ' << chessLayer->OpeningBookPanelViewStatus();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Board"))
		{
			if (ImGui::MenuItem("Copy Board", "Ctr+B"))
			{
				ImGui::SetClipboardText(ChessAPI::GetFEN().c_str());
			}
			if (ImGui::MenuItem("Flip Board", "Ctr+F"))
			{
				chessLayer->FlipBoard();
			}
			if (ImGui::MenuItem("Editor", "Ctr+E"))
			{
				chessLayer->OpenEditor();
			}
			if (ImGui::MenuItem("Go Next Move", "Right Arrow"))
			{
				ChessAPI::NextSavedMove();
			}
			if (ImGui::MenuItem("Go Previous Move", "Left Arrow"))
			{
				ChessAPI::PreviousSavedMove();
			}
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Open Default", "Ctr+Up Arrow"))
			{
				chessLayer->OpenChessEngine();
			}
			if (ImGui::MenuItem("Open"))
			{
				std::string filepath = Windows::Utils::OpenFile("Chess Engine (*.exe)\0*.exe\0");
				if (!filepath.empty())
					chessLayer->OpenChessEngine(filepath);
			}
			if (ImGui::MenuItem("Close", "Ctr+Down Arrow"))
			{
				chessLayer->CloseChessEngine();
			}
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