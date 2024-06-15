#include "OpeningBookPanel.h"

#include "imgui/imgui.h"
#include "Hazel/Core/Input.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Utils/PlatformUtils.h"

namespace Hazel
{
	OpeningBookPanel::OpeningBookPanel(ChessAPI::ChessAPI* chess)
		:m_chess(chess)
	{

	}

	void OpeningBookPanel::OnImGuiRender()
	{
		if (!m_viewPanel)
		{
			if (!m_chess->openBook.GetCobFilePath().empty())
				m_chess->openBook.CloseCOBfile();
			return;
		}

		ImGui::Begin("Opening Book", &m_viewPanel);

		ImGui::Text("Create New Opening Book");

		if (m_chess->openBook.IsReadyCreateThread())
		{
			ImGui::PushID("Create");
			ImGui::Button("Place Here Pgn File");
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::wstring wstr(path);
					std::string str(wstr.begin(), wstr.end());
					if (std::filesystem::path(str).extension() == ".pgn")
						m_chess->openBook.CreateCOBfile(str);
				}
				else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Database"))
				{
					m_chess->mainBoard.GetPgnFile()->operator[]((int)*(int*)payload->Data);
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string path = FileDialogs::OpenFile("Portable Game Notation (*.pgn)\0*.pgn\0");
				if (!path.empty())
					m_chess->openBook.CreateCOBfile(path);
			}
			ImGui::PopID();
		}

		ImGui::Separator();

		ImGui::NewLine();

		ImGui::PushID("Play");
		
		ImGui::Button("Place Here Cob File");
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::wstring wstr(path);
				std::string str(wstr.begin(), wstr.end());
				if(std::filesystem::path(str).extension() == ".cob")
					m_chess->openBook.OpenCOBfile(str);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			m_chess->openBook.CloseCOBfile();
			std::string path = FileDialogs::OpenFile("Chess Opening Book (*.cob)\0*.cob\0");
			if (!path.empty())
				m_chess->openBook.OpenCOBfile(path);
		}
		ImGui::PopID();

		if (!m_chess->openBook.GetCobFilePath().empty())
		{
			ImGui::TextWrapped(m_chess->openBook.GetCobFileName().c_str());
			if(ImGui::Button("Close"))
			{
				m_chess->openBook.CloseCOBfile();
			}
		}

		if (!Input::IsKeyPressed(Hazel::Key::Left) && !Input::IsKeyPressed(Hazel::Key::Right))
		{
			ImVec4 white = ImVec4(0.1, 0.79, 0.31, 1);
			ImVec4 draw = ImVec4(0.3, 0.58, 0.97, 1);
			ImVec4 black = ImVec4(0.79, 0.1, 0.1, 1);
			std::string move;
			auto posID = m_chess->mainBoard.GetFormatedPosition();
			const auto& moveptr = m_chess->openBook.GetOpeningBookMoves(posID);

			if (&moveptr && moveptr.size() && posID.size())
			{
				ImGui::BeginChild("Played Moves", ImVec2(0, 0), true);
				for (auto& moveob : moveptr)
				{
					ImGui::NewLine();

					float cursorStartPosY = ImGui::GetCursorPosY();
					ImGui::SetCursorPosY(cursorStartPosY + ImGui::GetTextLineHeight());
					ImGui::CalcItemWidth();
					if (ImGui::Button(moveob.strmove.c_str(), ImVec2(70, 0)))
						m_chess->mainBoard.GoMoveByStr(moveob.strmove);
					
					ImGui::SameLine();
					ImGui::SetCursorPosY(cursorStartPosY);
					//ImGui::Text("%d white won -- %d draw -- %d black won", i->second.whitewins, i->second.played - i->second.whitewins - i->second.blackwins, i->second.blackwins);

					float availx = ImGui::GetContentRegionAvail().x;
					int persentage[3] = { 100 * moveob.whitewins / moveob.played,100 * (moveob.played - moveob.blackwins - moveob.whitewins) / moveob.played,100 * moveob.blackwins / moveob.played };

					while (persentage[0] + persentage[1] + persentage[2] < 100)
					{
						persentage[1]++;
					}

					ImGui::PushStyleColor(ImGuiCol_Button, white);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, white);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, white);

					if (persentage[0] >= 8) { move = mtcs::trans_str(persentage[0]) + "%"; }
					else
						move = "##1";
					ImGui::Button(move.c_str(), ImVec2(availx * persentage[0] / 100, 0));

					ImGui::PopStyleColor(3);

					ImGui::PushStyleColor(ImGuiCol_Button, draw);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, draw);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, draw);

					if (persentage[1] >= 8) { move = mtcs::trans_str(persentage[1]) + "%"; }
					else
						move = "##2";
					if (persentage[1])
					{
						ImGui::SetCursorPosY(cursorStartPosY);
						ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - availx - 5 + availx * persentage[0] / 100 + 5);
						ImGui::Button(move.c_str(), ImVec2(availx * persentage[1] / 100, 0));
					}

					ImGui::PopStyleColor(3);

					ImGui::PushStyleColor(ImGuiCol_Button, black);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, black);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, black);

					if (persentage[2] >= 8) { move = mtcs::trans_str(persentage[2]) + "%"; }
					else
						move = "##3";
					if (persentage[2])
					{
						ImGui::SetCursorPosY(cursorStartPosY);
						ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - availx - 5 + availx * persentage[0] / 100 + availx * persentage[1] / 100 + 5);
						ImGui::Button(move.c_str(), ImVec2(availx * persentage[2] / 100, 0));
					}

					ImGui::PopStyleColor(3);

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.4, 0.5, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.4, 0.5, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3, 0.4, 0.5, 1));

					ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - availx);
					ImGui::Button(("Played " + mtcs::trans_str(moveob.played) + " Times").c_str(), ImVec2(availx, 0));

					ImGui::PopStyleColor(3);
				}
				ImGui::EndChild();
			}
		}
		
		ImGui::End();
	}


	bool& OpeningBookPanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}
