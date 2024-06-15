#include "EnginePanel.h"

#include "imgui/imgui.h"
#include "Hazel/Core/Input.h"
#include "Hazel/ImGui/ImGuiLayer.h"

namespace Hazel
{
	EnginePanel::EnginePanel(ChessAPI::ChessAPI* chess)
		:m_chess(chess)
	{

	}

	void EnginePanel::OnImGuiRender()
	{
		m_viewPanel = m_chess->engine.IsEngineOpen();
		if (!m_viewPanel)
			return;

		ImGui::Begin("Chess Engine", &m_viewPanel);
		
		//Title
		ImGui::TextWrapped(m_chess->engine.GetName().c_str());

		ImGui::Separator();
		//End-Title

		//Settings
		if (ImGui::TreeNode("Settings"))
		{
			ImGui::NewLine();

			if (ImGui::InputInt("Thread Count", &m_threadCount, 1, 1))
			{
				if (m_threadCount < 1) { m_threadCount = 1; }
				if (m_threadCount > std::thread::hardware_concurrency()) { m_threadCount = std::thread::hardware_concurrency(); }

				m_running = false;
				m_chess->engine.CommandChessEngine("stop");
				m_chess->engine.CommandChessEngine("setoption name Threads value " + mtcs::trans_str(m_threadCount));
				m_chess->engine.CommandChessEngine("setoption name Hash value " + mtcs::trans_str(m_hashMb));
				m_chess->engine.CommandChessEngine("ucinewgame");

			}
			if (ImGui::InputInt("Hash amount", &m_hashMb, 1, 1))
			{
				if (m_hashMb < 64) { m_hashMb = 64; }
				if (m_hashMb > 1024) { m_hashMb = 1024; }

				m_running = false;
				m_chess->engine.CommandChessEngine("stop");
				m_chess->engine.CommandChessEngine("setoption name Threads value " + mtcs::trans_str(m_threadCount));
				m_chess->engine.CommandChessEngine("setoption name Hash value " + mtcs::trans_str(m_hashMb));
				m_chess->engine.CommandChessEngine("ucinewgame");

			}

			ImGui::InputInt("Lines", &m_lines, 1, 1);
			if (m_lines < 1) { m_lines = 1; }
			if (m_lines > m_maxLines) { m_lines = m_maxLines; }

			ImGui::TreePop();
		}
		ImGui::Separator();
		//End-Settings


		//Moves

		auto& io = ImGui::GetIO();

		//blue(0.3, 0.58, 0.97) -> equal
		//red(0.79, 0.1, 0.1) -> black
		//green(0.1, 0.79, 0.31) -> white
		auto curBoard = m_chess->mainBoard.GetFormatedPosition();
		if (m_oldBoard != curBoard && m_running && !m_chess->mainBoard.IsWaitingForNewType()
			&& !Input::IsKeyPressed(Key::Right) && !Input::IsKeyPressed(Key::Left))
		{
			m_oldBoard = curBoard;
			m_chess->engine.CommandChessEngine("stop");

			m_chess->engine.CommandChessEngine(std::string("position fen ") + m_chess->mainBoard.GetFEN());
			m_chess->engine.CommandChessEngine("d");
			m_chess->engine.CommandChessEngine("go infinite");
		}

		m_winner = m_chess->engine.GetScore();

		ImVec4 color = ImVec4(0.3f, 0.58f, 0.97f, 1.0f);
		if (m_winner < -0.8) { color = ImVec4(0.79f, 0.1f, 0.1f, 1.0f); }
		else if (m_winner > 0.8) { color = ImVec4(0.1f, 0.79f, 0.31f, 1.0f); }

		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::PushFont(io.Fonts->Fonts[1]);

		if (std::abs(m_winner) > 1000.0f)
			ImGui::TextWrapped("Mate in %.0f", m_winner - 1000.0f * std::abs(m_winner) / m_winner);
		else if (std::abs(m_winner) == 1000.0f)
			ImGui::TextWrapped("Mated");
		else
			ImGui::TextWrapped("%.2f", m_winner);

		ImGui::PopFont();
		ImGui::PopStyleColor();

		ImGui::SameLine();

		float size = ImGui::GetFrameHeight();
		if (!m_running && ImGui::ImageButton((ImTextureID)m_IconPlay->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			m_running = true;
			//m_chess->engine.CommandChessEngine("setoption name Threads value " + mtcs::trans_str(m_threadCount));
			//m_chess->engine.CommandChessEngine("setoption name Hash value " + mtcs::trans_str(m_hashMb));
			//m_chess->engine.CommandChessEngine("ucinewgame");
			m_chess->engine.CommandChessEngine(std::string("position fen ") + m_chess->mainBoard.GetFEN());
			m_chess->engine.CommandChessEngine("d");
			m_chess->engine.CommandChessEngine("go infinite");
		}
		else if (m_running && ImGui::ImageButton((ImTextureID)m_IconStop->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			m_running = false;
			m_chess->engine.CommandChessEngine("stop");
		}

		ImGui::SameLine();

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[FontStyle::ExtraBold]);
		if (ImGui::Button("+", ImVec2(size, size)))
		{
			m_lines++;
			if (m_lines > m_maxLines) { m_lines = m_maxLines; }
		}

		ImGui::SameLine();

		if (ImGui::Button("-", ImVec2(size, size)))
		{
			m_lines--;
			if (m_lines < 1) { m_lines = 1; }
		}
		ImGui::PopFont();

		ImGui::SameLine();

		ImGui::Text("Depth: %d", m_chess->engine.GetDepth());

		ImGui::SameLine();

		ImGui::Text("Knps: %d", m_chess->engine.GetNodesPerSecond() / 1000);

		ImGui::BeginChild("Moves", ImVec2(0, 0), true);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		for (int i = 0; i < m_lines; i++)
		{
			ImGui::Text("%d:", i + 1);
			auto EngineMoves = m_chess->engine.GetBestMoveStr();
			int index = 0;
			for (int j = 0; j < EngineMoves.size() && m_running; j++)
			{
				ImGui::SameLine();
				if (ImGui::Button(EngineMoves[j].c_str()))
					index = j + 1;
			}
			for (int j = 0; j < index; j++)
				m_chess->mainBoard.GoMoveByStr(EngineMoves[j]);

			ImGui::Separator();
		}
		ImGui::PopStyleColor();

		ImGui::EndChild();
		//End-Moves

		if(!m_viewPanel)
		{
			m_chess->engine.CloseChessEngine();
		}
		ImGui::End();
	}

	void EnginePanel::Reset()
	{
		m_hashMb = 256;
		m_threadCount = 2;
		m_running = true;
		m_oldBoard.clear();
	}

	std::string EnginePanel::GetDefaultEngine() const
	{
		return m_defaultEngine;
	}

	void EnginePanel::SetDefaultEngine(const std::string& path)
	{
		m_defaultEngine = path;
	}

}
