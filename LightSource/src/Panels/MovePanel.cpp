#include "MovePanel.h"

#include "imgui.h"
#include "Walnut/UI/UI.h"
#include "Walnut/ApplicationGUI.h"

static bool s_IsVariationChecked = false;
static Chess::GameManager::MoveKey s_VariationKey;

static auto hoveredColor = ImVec4(0.2f, 0.48f, 0.87f, 1.0f);

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
				if(ChessAPI::GetCurPgnLabelValue("White") == "?")
					Walnut::UI::TextCentered("Unknown");
				else
					Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("White").c_str());
				
				if (ChessAPI::GetCurPgnLabelValue("WhiteElo") == "?")
					Walnut::UI::TextCentered("");
				else
					Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("WhiteElo").c_str());

				ImGui::PopStyleColor();
			}

			ImGui::EndChild();

			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4, 0.4, 0.4, 1));

			size = ImGui::CalcTextSize(ChessAPI::GetCurPgnLabelValue("Black").c_str()).y + ImGui::CalcTextSize(ChessAPI::GetCurPgnLabelValue("BlackElo").c_str()).y + 10;
			
			if (ImGui::BeginChild("Black", ImVec2(0, size)))
			{
				if (ChessAPI::GetCurPgnLabelValue("Black") == "?")
					Walnut::UI::TextCentered("Unknown");
				else
					Walnut::UI::TextCentered(ChessAPI::GetCurPgnLabelValue("Black").c_str());

				if (ChessAPI::GetCurPgnLabelValue("BlackElo") == "?")
					Walnut::UI::TextCentered("");
				else
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

		if (!ImGui::IsAnyItemHovered())
			m_HoveredMove.clear();

		if (ImGui::BeginTabBar("movebar"))
		{
			if (ImGui::BeginTabItem("Raw"))
			{
				ImGui::BeginChild("##DrawMoves", ImVec2(0, ImGui::GetContentRegionAvail().y - 7 * ImGui::GetStyle().ItemSpacing.y));

				WriteMove(m_moves, pathmove, 0);
				
				ImGui::EndChild();

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Training"))
			{
				auto& curMovePath = ChessAPI::GetMoveIntFormat();
				Chess::PgnGame::ChessMovesPath* ptrpgnMovePath = &m_moves;
				for (int i = 1; i < curMovePath.size(); i += 2)
				{
					ptrpgnMovePath = &ptrpgnMovePath->children[curMovePath[i]];
				}
				if (curMovePath[curMovePath.size() - 1] >= 0)
					ImGui::Text(ptrpgnMovePath->move[curMovePath[curMovePath.size() - 1]].c_str());
				ImGui::EndTabItem();
			}

			{
				ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 5 * ImGui::GetStyle().ItemSpacing.y);

				ImGui::Separator();

				ImGui::PushStyleColor(ImGuiCol_Text, hoveredColor);

				ImGui::Text(("ECO: " + ChessAPI::GetCurPgnLabelValue("ECO")).c_str());
				ImGui::SameLine();
				ImGui::Text("Blah blah blaah");

				ImGui::PopStyleColor();
			}

			ImGui::EndTabBar();
		}

		//for (auto& i : m_HoveredMove)
		//	std::cout << i << ' ';
		//std::cout << '\n';

		ImGui::End();
		
	}

	bool& MovePanel::IsPanelOpen()
	{
		return m_viewPanel;
	}

	void MovePanel::WriteMove(const Chess::PgnGame::ChessMovesPath& par, std::vector<int>& pathmove, float extrain)
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

				Chess::GameManager::MoveKey curVariationKey = pathmove;

				float prevCursotPosX = ImGui::GetCursorPosX();
				ImGui::SetCursorPosX(prevCursotPosX + extrain + 20);

				ImGui::NewLine();
				ImGui::SameLine();

				bool didICloseIt = false;

				if (s_IsVariationChecked)
				{
					didICloseIt = true;
					s_IsVariationChecked = false;

					ImGui::PopStyleColor();
				}

				std::string childID;
				{
					int j = 1;

					if (pathmove[1] < 24)
					{
						childID += (char)('A' + pathmove[1]);
						childID += '.';
						j = 3;
					}

					for (; j < pathmove.size(); j += 2)
					{
						childID += std::to_string(pathmove[j] + 1);
						childID += '.';
					}

					childID.pop_back();
				}

				{
					ImGui::PushID(childID.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));

					ImGui::Button(childID.c_str());

					ImGui::PopStyleColor(3);

					ImGui::PopID();
				}

				bool isHovered = ImGui::IsItemHovered() || s_VariationKey == curVariationKey;

				if (isHovered)
				{
					s_IsVariationChecked = true;
					s_VariationKey.clear();

					ImGui::PushStyleColor(ImGuiCol_Text, hoveredColor);
				}

				ImGui::SameLine();

				ImGui::Text("[ ");

				WriteMove(par.children[index], pathmove, extrain + 20);

				ImGui::SameLine();

				ImGui::Text("] ");

				if (isHovered)
				{
					s_IsVariationChecked = false;

					ImGui::PopStyleColor();
				}

				ImGui::SameLine();

				{
					ImGui::PushID(('s' + childID).c_str());
				
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
				
					ImGui::SmallButton(childID.c_str());
				
					if (ImGui::IsItemHovered())
						s_VariationKey = curVariationKey;
					
					ImGui::PopStyleColor(3);
				
					ImGui::PopID();
				}

				if (didICloseIt)
				{
					s_IsVariationChecked = true;

					ImGui::PushStyleColor(ImGuiCol_Text, hoveredColor);
				}

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
				
				if (pathmove == ChessAPI::GetMoveIntFormat()) { red = 0.5; green = 0.8; blue = 0.6; alfa = 0.7; }

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(red, green, blue, alfa));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(red, green, blue, alfa));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(red, green, blue, 0));

				auto hovered = ImGui::GetStyleColorVec4(ImGuiCol_Text);
				if (pathmove == m_HoveredMove)
					hovered = hoveredColor;

				ImGui::PushStyleColor(ImGuiCol_Text, hovered);

				auto io = ImGui::GetIO();
				ImFont* font = io.FontDefault;
				if (!par.parent)
					font = Walnut::Application::GetFont("Bold");
				else if (i == 0 || (i + 1 < par.move.size() && par.move[i + 1] == "child"))
					font = Walnut::Application::GetFont("Italic");

				ImGui::PushFont(font);

				std::string id = "";

				for (auto& i : pathmove)
				{
					id += std::to_string(i);
					id += '.';
				}

				ImGui::PushID(id.c_str());

				ImGui::SameLine();
				//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 6);
				if (ImGui::Button(par.move[i].c_str()))
				{
					ChessAPI::GoMoveByIntFormat(pathmove);
					ImGui::SetScrollHereY();
				}
				if (pathmove == ChessAPI::GetMoveIntFormat() && ChessAPI::IsBoardChanged() == true)
					ImGui::SetScrollHereY();
				if (ImGui::IsItemHovered())
					m_HoveredMove = pathmove;

				totalsize += ImGui::GetItemRectSize().x + style.FramePadding.x * 2.0f;

				ImGui::PopID();
				ImGui::PopFont();
				ImGui::PopStyleColor(4);

				std::string PopupID = "MovePopup";
				for (auto& i : pathmove)
					PopupID += (char)(i + 1);

				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					ImGui::OpenPopup(PopupID.c_str());

				if (ImGui::BeginPopup(PopupID.c_str()))
				{
					if (ImGui::Selectable("Delete Move"))
						ChessAPI::DeleteMove(pathmove);

					if (ImGui::Selectable("Delete Variation"))
						ChessAPI::DeleteVariation(pathmove);

					ImGui::BeginDisabled(pathmove.size() == 1);

					if (ImGui::Selectable("Promote Variation"))
						ChessAPI::PromoteVariation(pathmove);

					ImGui::EndDisabled();

					ImGui::EndPopup();
				}

				if (i == par.move.size() - 1) { return; }
			}
		}
	}
}
