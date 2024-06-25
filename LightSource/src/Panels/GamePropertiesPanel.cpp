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
			ImGui::InputText(name.c_str(), &ChessAPI::GetCurPgnLabelValue(name));
		}
		ImGui::Separator();
		if (!m_addinglabel)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			if (ImGui::Button("Add a new label"))
			{
				m_addinglabel = true;
			}
			ImGui::PopStyleColor();
		}
		if (m_addinglabel)
		{
			ImGui::InputText("Label Name", &m_nlabelname);
			auto& names = ChessAPI::GetCurPgnLabelNames();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			if (ImGui::Button("Add") && m_nlabelname != ""
				&& std::find(names.begin(), names.end(), m_nlabelname) == names.end())
			{
				ChessAPI::GetCurPgnLabelValue(m_nlabelname);
				m_nlabelname = "";
				m_addinglabel = false;
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Cansel"))
			{
				m_nlabelname = "";
				m_addinglabel = false;
			}
			ImGui::PopStyleColor();
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
