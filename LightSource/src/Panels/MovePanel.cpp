#include "MovePanel.h"

#include "imgui.h"
#include "Walnut/UI/UI.h"
#include "Walnut/ApplicationGUI.h"

namespace Panels
{
	void MovePanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Moves", &m_viewPanel);
		
		//ImGui::Text(std::to_string(ChessAPI::GetActiveGame()+1).c_str());

		ImGui::NewLine();

		ImGuiStyle& style = ImGui::GetStyle();

		if (ChessAPI::GetPgnFileName() == "New Game")
		{
			float size = ImGui::CalcTextSize("New Game").x + style.FramePadding.x * 2.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - size) * 0.5;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::TextWrapped("New Game");
		}
		else
		{
			//ImGui::TextWrapped(ChessAPI::GetPgnFileName().c_str());
			//ImGui::NewLine();

			ImGui::PushFont(Walnut::Application::GetFont("Bold"));

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.7, 0.7, 0.7, 1));
			float size = ImGui::CalcTextSize(ChessAPI::GetCurPgnLabelValue("White").c_str()).y + ImGui::CalcTextSize(ChessAPI::GetCurPgnLabelValue("WhiteElo").c_str()).y + 10;
			if (ImGui::BeginChild("White", ImVec2(0, size)))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(21, 21, 21, 255));
				Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("White").c_str());
				Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("WhiteElo").c_str());
				ImGui::PopStyleColor();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4, 0.4, 0.4, 1));
			size = ImGui::CalcTextSize(ChessAPI::GetCurPgnLabelValue("Black").c_str()).y + ImGui::CalcTextSize(ChessAPI::GetCurPgnLabelValue("BlackElo").c_str()).y + 10;
			if (ImGui::BeginChild("Black", ImVec2(0, size)))
			{
				Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("Black").c_str());
				Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("BlackElo").c_str());
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::PopFont();

			ImGui::Text(("ECO: " + ChessAPI::GetCurPgnLabelValue("ECO")).c_str());
			ImGui::SameLine();
			ImGui::Text(("Result: " + ChessAPI::GetCurPgnLabelValue("Result")).c_str());
		}
		ImGui::NewLine();

		ChessAPI::GetMovesPgnFormat(m_moves);

		std::vector<int> pathmove;
		pathmove.push_back(-1);


		if (ImGui::BeginTabBar("movebar"))
		{
			if (ImGui::BeginTabItem("Raw"))
			{
				ImGui::NewLine();
				ImGui::BeginChild("##DrawMoves");
				WriteMove(m_moves, pathmove, 0);
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Training"))
			{
				auto& curMovePath = ChessAPI::GetMoveIntFormat();
				chess::Pgn_Game::ChessMovesPath* ptrpgnMovePath = &m_moves;
				for (int i = 1; i < curMovePath.size(); i += 2)
				{
					ptrpgnMovePath = &ptrpgnMovePath->children[curMovePath[i]];
				}
				if (curMovePath[curMovePath.size() - 1] >= 0)
					ImGui::Text(ptrpgnMovePath->move[curMovePath[curMovePath.size() - 1]].c_str());
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
		
	}

	bool& MovePanel::IsPanelOpen()
	{
		return m_viewPanel;
	}

	void MovePanel::WriteMove(const chess::Pgn_Game::ChessMovesPath& par, std::vector<int>& pathmove, float extrain)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		size_t index = 0;
		float totalsize = 0;

		if (extrain)
			totalsize += ImGui::CalcTextSize("[ ").x + style.FramePadding.x * 2.0f + extrain;

		for (size_t i = 0; i < par.move.size(); i++)
		{
			pathmove[pathmove.size() - 1] += 1;
			if (par.move[i] == "child")
			{
				pathmove.push_back(index);
				pathmove.push_back(-1);

				float prevCursotPosX = ImGui::GetCursorPosX();
				ImGui::SetCursorPosX(prevCursotPosX + extrain + 20);

				ImGui::NewLine();
				ImGui::SameLine();

				ImGui::Text("[ ");

				WriteMove(par.children[index], pathmove, extrain + 20);

				ImGui::SameLine();

				ImGui::Text("] ");

				ImGui::SetCursorPosX(prevCursotPosX + extrain);
				ImGui::NewLine();

				pathmove.resize(pathmove.size() - 2);

				totalsize = 0;
				index += 1;
				continue;
			}
			else
			{
				if (i == par.move.size() - 1)
					totalsize += ImGui::CalcTextSize("] ").x + style.FramePadding.x * 2.0f;

				if (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() - totalsize < 88)
				{
					totalsize = extrain;
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + extrain);
					ImGui::NewLine();
				}

				float red = 0, green = 0, blue = 0, alfa = 0;
				//if (i == par.move.size() - 1 || i == 0 || par.move[i + 1] == "child") { red = 1; green = 204 / 255; blue = 203 / 255; alfa = 0.7; }

				if (extrain)
				{
					red = 1 * (1 - extrain / 100);
					green = 1 * (1 - extrain / 100);
					blue = 224 / 256 * (1 - extrain / 100);
					alfa = 0.4;
				}

				if (pathmove == ChessAPI::GetMoveIntFormat()) { red = 0.5; green = 0.8; blue = 0.6; alfa = 0.7; }

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(red, green, blue, alfa));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(red, green, blue, alfa));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(red, green, blue, 0));


				auto io = ImGui::GetIO();
				ImFont* font = io.FontDefault;
				if (!par.parent)
					font = Walnut::Application::GetFont("Bold");

				ImGui::PushFont(font);
				ImGui::PushID((par.move[i] + mtcs::trans_str(pathmove.size())).c_str());

				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 6);
				if (ImGui::Button(par.move[i].c_str()))
				{
					ChessAPI::GoMoveByIntFormat(pathmove);
					ImGui::SetScrollHereY();
				}
				if(pathmove == ChessAPI::GetMoveIntFormat() && ChessAPI::IsBoardChanged() == true)
					ImGui::SetScrollHereY();
					

				totalsize += ImGui::GetItemRectSize().x + 5;

				ImGui::PopID();
				ImGui::PopFont();
				ImGui::PopStyleColor(3);
				
				std::string PopupID = "MovePopup";
				for (auto& i : pathmove)
					PopupID += (char)(i + 1);

				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					ImGui::OpenPopup(PopupID.c_str());

				if (ImGui::BeginPopup(PopupID.c_str()))
				{
					if (ImGui::Selectable("Delete Move"))
					{
						ChessAPI::DeleteMove(pathmove);
					}
					if (ImGui::Selectable("Delete Variation"))
					{
						ChessAPI::DeleteVariation(pathmove);
					}
					if (ImGui::Selectable("Promote Variation"))
					{
						ChessAPI::PromoteVariation(pathmove);\
					}

					ImGui::EndPopup();
				}

				if (i == par.move.size() - 1) { return; }
			}
		}
	}

}
