#include "DatabasePanel.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"


namespace Panels
{
	DatabasePanel::DatabasePanel()
	{
		m_ecoItems.reserve(500);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 100; j++)
			{
				char label = 'A' + i;
				if (j < 10)
					m_ecoItems.emplace_back(label + ("0" + std::to_string(j)));
				else
					m_ecoItems.emplace_back(label + std::to_string(j));
			}
		}
	}

	void DatabasePanel::Reset()
	{
		m_searchSetting.clear();
		m_name_white = true;
		m_name_black = true;
		m_name_to_search.clear();
		m_eco_to_search.clear();
		m_ecoItems.clear();
		m_date_to_search.clear();
		m_search_resualt.clear();
		m_lastPointedRow = 0;
	}

	void DatabasePanel::OnImGuiRender()
	{
		if (m_filePath != ChessAPI::GetPgnFilePath())
		{
			Reset();
			m_filePath = ChessAPI::GetPgnFilePath();
		}

		chess::Pgn_File* pgnfile = ChessAPI::GetPgnFile();
		if (pgnfile)
		{
			ImGui::Begin("Database");
			ImGui::TextWrapped(ChessAPI::GetPgnFileName().c_str());
			
			ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
			ImGui::InputText("Name", &m_name_to_search);

			ImGui::SameLine();

			//ImGuiLayer::TextInputComboBox("ECO", m_eco_to_search, m_ecoItems, 3, ImGui::CalcTextSize("1234567").x);

			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::CalcTextSize(" dd/mm/yyyy ").x);
			ImGui::InputText("Date", &m_date_to_search);

			ImGui::Checkbox("White", &m_name_white);

			ImGui::SameLine();

			ImGui::Checkbox("Black", &m_name_black);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.8, 0.2, 0.75));
			if (ImGui::Button("Search"))
			{
				m_search_resualt.clear();
				m_lastPointedRow = 0;
				int a;
				if (m_name_white || !m_name_black)
				{
					m_searchSetting.clear();

					if (m_name_to_search != "")
						m_searchSetting["White"] = m_name_to_search;
					if (m_eco_to_search != "")
						m_searchSetting["ECO"] = m_eco_to_search;
					if (m_date_to_search != "")
						m_searchSetting["Date"] = m_date_to_search;

					pgnfile->SearchLabel(m_search_resualt, a, m_searchSetting);
				}
				if (m_name_black || !m_name_white)
				{
					m_searchSetting.clear();

					if (m_name_to_search != "")
						m_searchSetting["Black"] = m_name_to_search;
					if (m_eco_to_search != "")
						m_searchSetting["ECO"] = m_eco_to_search;
					if (m_date_to_search != "")
						m_searchSetting["Date"] = m_date_to_search;

					pgnfile->SearchLabel(m_search_resualt, a, m_searchSetting);
				}
			}
			ImGui::PopStyleColor(2);

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8, 0.2, 0.2, 0.75));
			if (ImGui::Button("Clear"))
			{
				m_search_resualt.clear();
				m_name_to_search = "";
				m_eco_to_search = "";
			}
			ImGui::PopStyleColor(2);

			ImGui::NewLine();
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 1.0f));

			if (ImGui::Button("New Game"))
			{
				ChessAPI::NewGameInFile();
				m_lastPointedRow = pgnfile->GetSize() - 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Game"))
			{
				ChessAPI::DeleteGameInFile(m_lastPointedRow);
				m_lastPointedRow = 0;
			}

			ImGui::PopStyleColor();

			ImGui::Separator();

			int all = pgnfile->GetSize();
			int showed = 1000;
			float maxscrollpos;
			float scrollpos;
			int start;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());

			if (ImGui::BeginTable("table_scrollx", 9, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
				ImGui::TableSetupColumn(m_important_prop[0].c_str(), ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
				for (int i = 1; i < m_important_prop.size() - 2; i++)
					ImGui::TableSetupColumn(m_important_prop[i].c_str());
				ImGui::TableHeadersRow();

				//TODO: Sorting
				//if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
				//{
				//	if (sorts_specs->SpecsDirty)
				//	{
				//		if (start > 0)
				//			
				//		MyItem::s_current_sort_specs = NULL;
				//		sorts_specs->SpecsDirty = false;
				//	}
				//}

				maxscrollpos = ImGui::GetScrollMaxY();
				scrollpos = ImGui::GetScrollY();

				if (m_search_resualt.empty())
				{
					start = all * (scrollpos / maxscrollpos) - showed;
					if (start < 0)
						start = 0;
					for (int row = start; row < start + showed && row < all; row++)
					{
						ImGui::TableNextRow();

						if (ChessAPI::GetActiveGame() == m_lastPointedRow && row == m_lastPointedRow)
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 80, 80, 255));
						else if (row == m_lastPointedRow)
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
						else if (row == ChessAPI::GetActiveGame())
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 110, 50, 255));

						for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
						{
							if (!ImGui::TableSetColumnIndex(column) && column > 0)
								continue;
							if (column == 0)
								ImGui::Text("Line %d", row + 1);
							else
							{
								ImGui::PushID(row * ImGui::TableGetColumnCount() + column);
								if (ImGui::Selectable(pgnfile->operator[](row)[m_important_prop[column - 1]].c_str()))
								{
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
									m_lastPointedRow = row;
								}
								if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
									ChessAPI::OpenChessGameInFile(row);

								if (ImGui::BeginDragDropSource())
								{
									ImGui::SetDragDropPayload("Database", &row, sizeof(int));
									ImGui::EndDragDropSource();
								}
								ImGui::PopID();
							}
						}
					}
				}
				else
				{
					start = m_search_resualt.size() * (scrollpos / maxscrollpos) - showed;
					all = m_search_resualt.size();
					if (start < 0)
						start = 0;
					for (int i = start; i < start + showed && i < all; i++)
					{
						ImGui::TableNextRow();

						if (ChessAPI::GetActiveGame() == m_lastPointedRow && m_search_resualt[i] == m_lastPointedRow)
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 80, 80, 255));
						else if (m_search_resualt[i] == m_lastPointedRow)
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
						else if(m_search_resualt[i] == ChessAPI::GetActiveGame())
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 110, 50, 255));

						for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
						{
							if (!ImGui::TableSetColumnIndex(column) && column > 0)
								continue;
							if (column == 0)
								ImGui::Text("Line %d", m_search_resualt[i] + 1);
							else
							{
								ImGui::PushID(i * ImGui::TableGetColumnCount() + column);
								if (ImGui::Selectable(pgnfile->operator[](m_search_resualt[i])[m_important_prop[column - 1]].c_str()))
								{
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
									m_lastPointedRow = m_search_resualt[i];
								}
								if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
									ChessAPI::OpenChessGameInFile(m_search_resualt[i]);
								if (ImGui::BeginDragDropSource())
								{
									ImGui::SetDragDropPayload("Database", &m_search_resualt[i], sizeof(int));
									ImGui::EndDragDropSource();
								}

								ImGui::PopID();
							}
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleColor(3);

			ImGui::End();
		}
		else
		{
			m_lastPointedRow = 0;
			m_eco_to_search = "";
			m_name_to_search = "";
			m_search_resualt.clear();
		}
	}

}
