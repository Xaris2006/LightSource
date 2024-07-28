#include "OpeningBookPanel.h"

#include "imgui.h"

#include "ChessAPI.h"
#include "../Windows/WindowsUtils.h"

#include <filesystem>

namespace Panels
{
	OpeningBookPanel::~OpeningBookPanel()
	{
		m_EndPlayThreadJob = true;
		if (m_PlayThread)
			m_PlayThread->join();

		delete m_CreateThread;
		delete m_PlayThread;
		delete m_OpeningBook;
	}

	void OpeningBookPanel::OnImGuiRender()
	{
		if (!m_viewPanel)
		{
			if (m_cobPath.empty())
				CloseCOBfile();
			return;
		}

		ImGui::Begin("Opening Book", &m_viewPanel);


		if (IsReadyCreateThread())
		{
			ImGui::Text("Create New Opening Book");
			ImGui::NewLine();
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
						CreateCOBfile(str);
				}
				else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Database"))
				{
					ChessAPI::GetPgnFile()->operator[]((int)*(int*)payload->Data);
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string path = Windows::Utils::OpenFile("Portable Game Notation (*.pgn)\0*.pgn\0");
				if (!path.empty())
					CreateCOBfile(path);
			}
			ImGui::PopID();
		}
		else
		{
			ImGui::Text("A cob file is currently being created");

			//ImGui::SliderInt("Status", &m_Status, 0, 100, "%d%%", ImGuiSliderFlags_NoInput);
			
			auto ycursor = ImGui::GetCursorPosY();
			auto xcursor = ImGui::GetCursorPosX();

			float availx = 0.6*ImGui::GetContentRegionAvail().x;
			ImGui::Button("##end", ImVec2(availx, 0));
			
			ImGui::SameLine();
			ImGui::Text("Status");

			ImGui::SetCursorPosY(ycursor);
			ImGui::SetCursorPosX(xcursor);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));

			ImGui::Button((std::to_string(m_Status)+'%').c_str(), ImVec2(availx * m_Status / 100, 0));
			
			ImGui::PopStyleColor(3);

			//ImGui::ProgressBar((float)m_Status/100.0f, ImVec2(0.0f, 0.0f));
			//ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			//ImGui::Text("Status");
		}

		ImGui::Separator();

		ImGui::NewLine();

		ImGui::PushID("Play");
		
		ImGui::Text("Open An Opening Book");
		ImGui::NewLine();
		ImGui::Button("Place Here Cob File");
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::wstring wstr(path);
				std::string str(wstr.begin(), wstr.end());
				if(std::filesystem::path(str).extension() == ".cob")
					OpenCOBfile(str);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			CloseCOBfile();
			std::string path = Windows::Utils::OpenFile("Chess Opening Book (*.cob)\0*.cob\0");
			if (!path.empty())
				OpenCOBfile(path);
		}
		ImGui::PopID();

		if (!m_cobPath.empty())
		{
			ImGui::TextWrapped(m_cobFilename.c_str());
			if(ImGui::Button("Close"))
			{
				CloseCOBfile();
			}
		}

		if (!ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && !ImGui::IsKeyPressed(ImGuiKey_RightArrow))
		{
			ImVec4 white = ImVec4(0.1, 0.79, 0.31, 1);
			ImVec4 draw = ImVec4(0.3, 0.58, 0.97, 1);
			ImVec4 black = ImVec4(0.79, 0.1, 0.1, 1);
			std::string move;
			auto posID = ChessAPI::GetFormatedPosition();
			const auto& moveptr = GetOpeningBookMoves(posID);

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
						ChessAPI::GoMoveByStr(moveob.strmove);
					
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


	void OpeningBookPanel::CreateCOBfile(const std::string& pgnfilepath)
	{
		m_Status = 0;
		m_IsCreateThreadEnd = false;

		m_CreateThread = new std::thread(
			[this, pgnfilepath]()
			{
				chess::OpeningBook::CreateCOBByPGN(pgnfilepath, m_Status);
				m_IsCreateThreadEnd = true;
			}
		);
	}

	void OpeningBookPanel::OpenCOBfile(const std::string& filepath)
	{
		if (filepath == m_cobPath) { return; }

		delete m_OpeningBook;

		m_cobPath = filepath;

		auto nameWithExtension = std::filesystem::path(filepath).filename();
		m_cobFilename = nameWithExtension.string().substr(0, nameWithExtension.string().find_last_of('.'));

		m_EndPlayThreadJob = false;
		m_PlayThread = new std::thread(
			[this, filepath]()
			{
				m_OpeningBook = new chess::OpeningBook(filepath);

				while (true)
				{
					if (m_EndPlayThreadJob)
						return;

					using namespace std::chrono_literals;
					std::this_thread::sleep_for(0.2s);

					if (m_CurPosition.empty())
						continue;
					m_Moves = m_OpeningBook->GetMovesByPosition(m_CurPosition, chess::OpeningBook::MOSTPLAYED);
				}
			}
		);
	}
	bool OpeningBookPanel::CloseCOBfile()
	{
		if (!m_EndPlayThreadJob)
		{
			m_EndPlayThreadJob = true;
			m_PlayThread->join();
			delete m_PlayThread;
			m_PlayThread = nullptr;

			delete m_OpeningBook;
			m_OpeningBook = nullptr;
			m_cobFilename = "";
			m_cobPath = "";

			m_CurPosition.clear();
			m_Moves.clear();

			return true;
		}
		return false;
	}

	std::vector<chess::OpeningBook::MoveOB>& OpeningBookPanel::GetOpeningBookMoves(const chess::OpeningBook::PositionID& posID)
	{
		if (!m_OpeningBook) { return m_Moves; }
		m_CurPosition = posID;
		return m_Moves;
	}

	int OpeningBookPanel::IsReadyCreateThread()
	{
		if (m_IsCreateThreadEnd)
		{
			if (m_CreateThread)
			{
				m_CreateThread->join();
				delete m_CreateThread;
				m_CreateThread = nullptr;
			}
			return true;
		}
		return false;
	}

	bool& OpeningBookPanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}
