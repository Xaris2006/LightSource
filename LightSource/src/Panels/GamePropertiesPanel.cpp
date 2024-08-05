#include "GamePropertiesPanel.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace Panels
{
	void GamePropertiesPanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Game Properties", &m_viewPanel);
		
		for (auto& name : ChessAPI::GetCurPgnLabelNames())
		{
			ImGui::PushID(&name);

			ImGui::InputText(name.c_str(), &ChessAPI::GetCurPgnLabelValue(name));

			//not worknig well
#if 0
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			if (ImGui::Button("Remove"))
				ChessAPI::GetPgnGame()->RemoveLabel(name);

			ImGui::PopStyleColor(3);
#endif
			ImGui::PopID();
		}
		ImGui::Separator();
		if (!m_addinglabel)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
			if (ImGui::Button("Add a new label"))
			{
				m_addinglabel = true;
			}
			ImGui::PopStyleColor(3);
		}
		if (m_addinglabel)
		{
			ImGui::InputText("Label Name", &m_nlabelname);
			auto& names = ChessAPI::GetCurPgnLabelNames();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
			if (ImGui::Button("Add") && m_nlabelname != ""
				&& std::find(names.begin(), names.end(), m_nlabelname) == names.end())
			{
				ChessAPI::GetCurPgnLabelValue(m_nlabelname);
				m_nlabelname = "";
				m_addinglabel = false;
			}
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
			if (ImGui::Button("Cansel"))
			{
				m_nlabelname = "";
				m_addinglabel = false;
			}
			ImGui::PopStyleColor(3);
		}
		ImGui::End();
		
	}

	void GamePropertiesPanel::Reset()
	{
		m_addinglabel = false;
		m_nlabelname = "";
	}

	bool& GamePropertiesPanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}
