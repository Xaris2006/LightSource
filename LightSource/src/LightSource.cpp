#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "Windows/WindowsUtils.h"

#include "ChessAPI.h"

#include "ImGuiBoard.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/DatabasePanel.h"
#include "Panels/GamePropertiesPanel.h"
#include "Panels/NotePanel.h" //will be included in Move panel
#include "Panels/MovePanel.h"

#include <iostream>
#include <array>

#include "GLFW/glfw3.h"

std::string g_AppDirectory;

class ChessLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());

		ChessAPI::Init();

		m_ChessBoard.OnAttach();
	}

	virtual void OnUIRender() override
	{		
		ImGui::ShowDemoWindow();

		UI_DrawAboutModal();

		m_ContentBrowserPanel.OnImGuiRender();
		m_ChessBoard.OnUIRender();
		m_DatabasePanel.OnImGuiRender();
		m_GamePropertiesPanel.OnImGuiRender();
		m_NotePanel.OnImGuiRender();
		m_MovePanel.OnImGuiRender();
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

	void FlipBoard()
	{
		m_ChessBoard.FlipBoard();
	}

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	static void New()
	{
		ChessAPI::SetNewChessGame("");
	}

	static void Open()
	{
		std::string filepath = Windows::Utils::OpenFile("Chess Database (*.pgn)\0*.pgn\0");
		if (!filepath.empty())
			ChessAPI::SetNewChessGame(filepath);
	}

	static void SaveAs()
	{
		std::string filepath = Windows::Utils::SaveFile("Chess Database (*.pgn)\0*.pgn\0");
		if (!filepath.empty())
		{
			ChessAPI::OverWriteChessFile(filepath);
			ChessAPI::SetNewChessGame(filepath);
		}
	}

	static void Save()
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

	bool m_AboutModalOpen = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Light Source";
	spec.CustomTitlebar = true;
	spec.IconPath = "lsb.png";
	//g_AppDirectory = std::filesystem::path(argv[0]).parent_path().string();
	g_AppDirectory = "E:\\programs\\c++\\Light-Source\\final4\\LightSource\\LightSource";

	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<ChessLayer> chessLayer = std::make_shared<ChessLayer>();
	app->PushLayer(chessLayer);
	app->SetMenubarCallback([app, chessLayer]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctr+N"))
			{
				ChessLayer::New();
			}
			if (ImGui::MenuItem("Open", "Ctr+O"))
			{
				ChessLayer::Open();
			}
			if (ImGui::MenuItem("Save", "Ctr+S"))
			{
				ChessLayer::Save();
			}
			if (ImGui::MenuItem("Save As", "Ctr+Shift+S"))
			{
				ChessLayer::SaveAs();
			}
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Content Browser", 0, &chessLayer->ContentBrowserPanelViewStatus())){}
			if (ImGui::MenuItem("Game Properties", 0, &chessLayer->GamePropertiesPanelViewStatus())){}
			if (ImGui::MenuItem("Notes", 0, &chessLayer->NotePanelViewStatus())){}
			if (ImGui::MenuItem("Moves", 0, &chessLayer->MovePanelViewStatus())){}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Board"))
		{
			if (ImGui::MenuItem("Flip Board", "Ctr+F"))
			{
				chessLayer->FlipBoard();
			}
			if (ImGui::MenuItem("Editor", "Ctr+E"))
			{
				
			}
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Open"))
			{

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