#include "ProfilePanel.h"

#include "Walnut/UI/UI.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <string>

namespace Panels
{
	ProfilePanel::ProfilePanel()
	{
		m_ProfileIcon = std::make_shared<Walnut::Image>("LightSourceApp\\Resources\\LightSource\\lsOn.png");
	}

	void ProfilePanel::OnImGuiRender()
	{
		ImGui::Begin("Profile");

		if (true)//internet is required
		{
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.79f, 0.1f, 0.1f, 1.0f));
			Walnut::UI::TextCentered("Internet is required!");
			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::BeginTable("table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginChild("Main");

			ImGui::BeginTabBar("ProfileMenu");

			if (ImGui::BeginTabItem("Profile"))
			{
				ImGui::NewLine();

				float childSizeX = ImGui::GetContentRegionAvail().x * 9 / 10;

				ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - childSizeX) / 2);
				ImGui::BeginChild("Activity", ImVec2(childSizeX, ImGui::GetContentRegionAvail().y / 2), true);

				//ImGui::Text("Active Hours: ");
				//ImGui::SameLine();
				//ImGui::Text("8pm - 2am");

				Walnut::UI::TextCentered("Active Hours: 8pm - 2am");

				ImGui::NewLine();

				static constexpr char* days[7] = { " S", " M", " T", " W", " T", " F", " S" };
				static constexpr char* months[12] = { "Jenouary", "February", "March", "April", "May", "June", "July", "Augoust",
				"Semptember", "October", "Noveber", "Deseber" };

				float tableSize = 895.0f;//ImGui::GetContentRegionAvail().x * 2 / 3;
				ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 895.0f)/2);

				if (ImGui::BeginTable("Activity", 29, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit,
					ImVec2(tableSize, 0)))
				{
					ImGui::TableSetupColumn("##Months");
					for (int i = 0; i < 4 * 7; i++)
					{
						ImGui::TableSetupColumn((days[i - (i / 7) * 7]));
					}
					ImGui::TableHeadersRow();

					for (int i = 0; i < 12; i++)
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text(months[i]);

						for (int j = 1; j < 4 * 7 + 1; j++)
						{
							ImGui::TableSetColumnIndex(j);

							ImVec4 ButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];

							if (j > 10)
								ButtonColor = ImVec4(0.3 * 0.5, 0.58 * 0.5, 0.97 * 0.5, 1);
							if (j > 20)
								ButtonColor = ImVec4(0.3 * 0.75, 0.58 * 0.75, 0.97 * 0.75, 1);
							if (j > 25)
								ButtonColor = ImVec4(0.3, 0.58, 0.97, 1);
							
							ImVec4 HAButtonColor = ButtonColor;
							HAButtonColor.w *= 0.7;
							
							ImGui::PushID(i * 29 + j);

							ImGui::PushStyleColor(ImGuiCol_Button, ButtonColor);
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HAButtonColor);
							ImGui::PushStyleColor(ImGuiCol_ButtonActive,  HAButtonColor);

							ImGui::Button("##A", ImVec2(ImGui::CalcTextSize("A").y, ImGui::CalcTextSize("A").y));

							if(ImGui::IsItemHovered())
								ImGui::SetTooltip("At this day {Name} played %i games", j);

							ImGui::PopStyleColor(3);
							
							ImGui::PopID();
						}
					}
					
					ImGui::EndTable();
				}

				ImVec2 size = ImGui::GetItemRectSize();

				ImGui::EndChild();

				ImGui::NewLine();

				ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - childSizeX) / 2);
				ImGui::BeginChild("ELO", ImVec2(childSizeX, 0), true);

				ImGui::NewLine();

				ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - size.x) / 2);

				const float values[6] = { 1400.0f, 1200.0f, 1870.0f, 1760.0f, 2430.0f, 2413.0f };
				ImGui::PlotHistogram("##values", values, IM_ARRAYSIZE(values), 0, "ELO", 1000.0f, 3000.0f, size);
				
				//ImPlot?????

				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Statistics"))
			{
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("Latest Games"))
			{
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("Chat"))
			{
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
			
			ImGui::EndChild();

			ImGui::TableSetColumnIndex(1);

			float size = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) * 0.8f;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size) * 0.5f);
			ImGui::Image((ImTextureID)m_ProfileIcon->GetRendererID(), ImVec2(size, size));

			ImGui::NewLine();
			ImGui::NewLine();
			
			ImGui::Separator();
			
			static std::string name = "";
			ImGui::InputText("Name", &name);
			
			static std::string email = "";
			ImGui::InputText("Email", &email);
			
			ImGui::Separator();
			
			static int fideID;
			ImGui::InputInt("Fide ID", &fideID);
			
			static int fideELO;
			ImGui::InputInt("Fide ELO", &fideELO);
			
			ImGui::Separator();
			
			static std::string lichessName = "";
			ImGui::InputText("Lichess Name", &lichessName);
			
			static int lichessELO;
			ImGui::InputInt("Lichess ELO", &lichessELO);
			
			ImGui::Separator();
			
			static std::string chesscomName = "";
			ImGui::InputText("Chess.com Name", &chesscomName);
			
			static int chesscomELO;
			ImGui::InputInt("Chess.com ELO", &chesscomELO);
			
			ImGui::Separator();
			
			ImGui::EndTable();
		}

		ImGui::End();
	}

}