#include "EnginePanel.h"

#include "ChessAPI.h"
#include "ChessCore/GameManager.h"
#include "imgui.h"
#include "../Source/Walnut/Application.h"

#include <mutex>
#include <thread>
#include <string>
#include <fstream>
#include <format>

#include "../../Walnut/Source/Walnut/Timer.h"

#include "../windowsMain.h"

//for Bar
float g_ChessEngineValue;
bool g_ChessEngineOpen;

static std::mutex s_vectorMutex;
static std::mutex s_moveMutex;

static Walnut::Timer s_time;
static Walnut::Timer s_timeScore;

static int FindLastOf(const std::string& source, const std::string& target)
{
	int size = target.
		size() - 1;
	int index = size;
	for (int i = source.size() - 1; i > -1; i--)
	{
		if (source[i] == target[index])
		{
			index--;
			if (index == -1)
				return i;
		}
		else if (index != size)
			index = size;
	}
	return -1;
}

namespace Panels
{
	EnginePanel::EnginePanel()
	{
		std::ifstream inFile("MyDocuments\\engines\\settings.ce");

		inFile >> m_threadCount;
		inFile >> m_hashMb;
		inFile >> m_lines;
		inFile >> m_SkillLevel;
		inFile >> m_SyzygyPath;
		inFile >> m_Syzygy50MoveRule;
		inFile >> m_LimitStrength;
		inFile >> m_Elo;
		inFile >> m_DefaultEngine;

		if (m_SyzygyPath == "False")
			m_SyzygyPath = "";

		if (m_DefaultEngine == "False")
			m_DefaultEngine = "";

		inFile.close();
	}

	EnginePanel::~EnginePanel()
	{
		CloseEngine();
	}

	void EnginePanel::OnImGuiRender()
	{

		if (ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_RightArrow))// || ImGui::IsMouseDown(ImGuiMouseButton_Left))
			s_timeScore.Reset();

		{
			m_AvailEngines.clear();

			std::ifstream inFile("MyDocuments\\engines\\Avail.ce");

			while (inFile.good())
			{
				std::string nEngine;
				inFile >> nEngine;
				int extensionIndex = nEngine.find_last_of('.');
				if (extensionIndex != std::string::npos)
					nEngine.erase(extensionIndex);
				m_AvailEngines.emplace_back(nEngine);
			}

			inFile.close();
		}

		m_viewPanel = IsEngineOpen();
		if (!m_viewPanel)
			return;

		ImGui::Begin("Chess Engine", &m_viewPanel);
		
		if (!m_viewPanel)
			CloseEngine();

		//Title
		ImGui::TextWrapped(GetName().c_str());

		ImGui::Separator();
		//End-Title

		//Settings

		static bool barCheckBox = false;
		barCheckBox = g_ChessEngineOpen;

		//if (ImGui::TreeNode("Settings"))
		//{
		//	ImGui::NewLine();
		//
		//	if (ImGui::InputInt("Thread Count", &m_threadCount, 1, 1))
		//	{
		//		if (m_threadCount < 1) { m_threadCount = 1; }
		//		if (m_threadCount > std::thread::hardware_concurrency()) { m_threadCount = std::thread::hardware_concurrency(); }
		//
		//		m_running = false;
		//		CommandChessEngine("stop");
		//		CommandChessEngine("setoption name Threads value " + std::to_string(m_threadCount));
		//		CommandChessEngine("setoption name Hash value " + std::to_string(m_hashMb));
		//		//CommandChessEngine("ucinewgame");
		//
		//	}
		//	if (ImGui::InputInt("Hash amount", &m_hashMb, 1, 1))
		//	{
		//		if (m_hashMb < 64) { m_hashMb = 64; }
		//		if (m_hashMb > 1024) { m_hashMb = 1024; }
		//
		//		m_running = false;
		//		CommandChessEngine("stop");
		//		CommandChessEngine("setoption name Threads value " + std::to_string(m_threadCount));
		//		CommandChessEngine("setoption name Hash value " + std::to_string(m_hashMb));
		//		//CommandChessEngine("ucinewgame");
		//
		//	}
		//	
		//	ImGui::Checkbox("Evaluation Bar", &barCheckBox);
		//
		//	ImGui::TreePop();
		//}


		ImGui::Checkbox("Evaluation Bar", &barCheckBox);

		ImGui::Separator();

		g_ChessEngineOpen = barCheckBox;

		//End-Settings


		//Moves

		auto& io = ImGui::GetIO();

		//blue(0.3, 0.58, 0.97) -> equal
		//red(0.79, 0.1, 0.1) -> black
		//green(0.1, 0.79, 0.31) -> white
		auto curBoard = ChessAPI::GetFormatedPosition();
		if (m_oldBoard != curBoard && m_running && !ChessAPI::IsWaitingForNewType()
			&& s_timeScore.Elapsed() > 0.2)
		{
			m_oldBoard = curBoard;
			CommandChessEngine("stop");

			CommandChessEngine(std::string("position fen ") + ChessAPI::GetFEN());
			CommandChessEngine("go infinite");
		}

		ImVec4 color = ImVec4(0.3f, 0.58f, 0.97f, 1.0f);
		if (m_Score[0] < -0.8) { color = ImVec4(0.79f, 0.1f, 0.1f, 1.0f); }
		else if (m_Score[0] > 0.8) { color = ImVec4(0.1f, 0.79f, 0.31f, 1.0f); }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 0.4f));

		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::PushFont(io.Fonts->Fonts[1]);

		float buSize = ImGui::CalcTextSize("1234567").x;

		if (std::abs(m_Score[0]) > 1000.0f)
			ImGui::Button(std::format("{0}#", m_Score[0] - 1000.0f * std::abs(m_Score[0]) / m_Score[0]).c_str(), ImVec2(buSize, 0));
		else if (std::abs(m_Score[0]) == 1000.0f)
			ImGui::Button("Mated", ImVec2(buSize, 0));
		else
			ImGui::Button(std::format("{0}", m_Score[0]).c_str(), ImVec2(buSize, 0));
		
		g_ChessEngineValue = m_Score[0];

		ImGui::PopFont();
		ImGui::PopStyleColor(4);

		ImGui::SameLine();

		float size = ImGui::GetFrameHeight();
		if (!m_running && ImGui::ImageButton((ImTextureID)m_IconPlay->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			m_running = true;
			CommandChessEngine(std::string("position fen ") + ChessAPI::GetFEN());
			CommandChessEngine("go infinite");
		}
		else if (m_running && ImGui::ImageButton((ImTextureID)m_IconStop->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			m_running = false;
			CommandChessEngine("stop");
		}

		ImGui::SameLine();

		ImGui::PushFont((Walnut::Application::GetFont("Bold")));
		if (ImGui::Button("+", ImVec2(size, size)))
		{
			m_lines++;
			if (m_lines > m_maxLines) { m_lines = m_maxLines; }

			CommandChessEngine("stop");
			CommandChessEngine("setoption name MultiPV value " + std::to_string(m_lines));
			if(m_running)
				CommandChessEngine("go infinite");

		}

		ImGui::SameLine();

		if (ImGui::Button("-", ImVec2(size, size)))
		{
			m_lines--;
			if (m_lines < 1) { m_lines = 1; }

			CommandChessEngine("stop");
			CommandChessEngine("setoption name MultiPV value " + std::to_string(m_lines));
			if(m_running)
				CommandChessEngine("go infinite");
		}
		ImGui::PopFont();

		ImGui::SameLine();

		ImGui::Text("Depth: %d", GetDepth());

		ImGui::SameLine();

		ImGui::Text("Knps: %d", GetNodesPerSecond() / 1000);

		ImGui::BeginChild("Moves", ImVec2(0, 0), true);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		for (int i = 0; i < m_lines; i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.58f, 0.97f, 1.0f));

			if (std::abs(m_Score[i]) > 1000.0f)
				ImGui::TextWrapped("Mate in %.0f", m_Score[i] - 1000.0f * std::abs(m_Score[i]) / m_Score[i]);
			else if (std::abs(m_Score[i]) == 1000.0f)
				ImGui::TextWrapped("Mated");
			else
				ImGui::TextWrapped("%.2f", m_Score[i]);

			ImGui::PopStyleColor();

			std::vector<std::string> EngineMoves;
			GetBestMoveStr(i, EngineMoves);
			
			int index = 0;
			for (int j = 0; j < EngineMoves.size() && m_running && std::abs(m_Score[0]) != 1000.0f; j++)
			{
				ImGui::SameLine();

				if (EngineMoves[j].empty())
					break;

				ImGui::PushID((EngineMoves[j] + std::to_string(i*10 + j)).c_str());

				if (ImGui::Button(EngineMoves[j].c_str()))
					index = j + 1;

				ImGui::PopID();
			}

			if (index && s_time.Elapsed() > 0.5)
			{
				for (int j = 0; j < index; j++)
					ChessAPI::GoMoveByStr(EngineMoves[j]);
				s_time.Reset();
			}
			
			ImGui::Separator();
		}
		ImGui::PopStyleColor();

		ImGui::EndChild();
		//End-Moves

		if(!m_viewPanel)
		{
			CloseEngine();
		}
		ImGui::End();
	}

	void EnginePanel::Reset()
	{
		m_running = false;
		m_oldBoard.clear();

		{
			std::ifstream inFile("MyDocuments\\engines\\settings.ce");

			inFile >> m_threadCount;
			inFile >> m_hashMb;
			inFile >> m_lines;
			inFile >> m_SkillLevel;
			inFile >> m_SyzygyPath;
			inFile >> m_Syzygy50MoveRule;
			inFile >> m_LimitStrength;
			inFile >> m_Elo;
			inFile >> m_DefaultEngine;

			if (m_SyzygyPath == "False")
				m_SyzygyPath = "";

			if (m_DefaultEngine == "False")
				m_DefaultEngine = "";

			inFile.close();
		}
	}

	std::string EnginePanel::GetDefaultEngine() const
	{
		return "MyDocuments\\engines\\" + m_DefaultEngine;
	}

	void EnginePanel::OpenEngine(const std::string& programpath)
	{
		if (programpath.empty())
			return;

		if (IsEngineOpen())
			CloseEngine();

		g_ChessEngineOpen = true;
		m_running = true;

		m_processThread = new std::thread(
			[this, programpath]()
			{
				Process EngineApp(std::wstring(programpath.begin(), programpath.end()), L"");
				m_EngineName = std::filesystem::path(programpath).filename().string().substr(0, std::filesystem::path(programpath).filename().string().find_last_of('.'));

				m_EndThread = false;

				if (!EngineApp.IsProcessActive())
				{
					m_EndThread = true;
					return;
				}

				EngineApp.Write("uci");
				bool uciokFinded = false;

				for (int i = 0; i < 10; i++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					std::string info = EngineApp.Read();

					if (info.find("uciok") + 1)
					{
						uciokFinded = true;
						break;
					}
				}

				if (!uciokFinded)
				{
					m_EndThread = true;
					return;
				}

				EngineApp.Write("setoption name Threads value " + std::to_string(m_threadCount));
				EngineApp.Write("setoption name Hash value " + std::to_string(m_hashMb));

				std::vector<std::string> StockMoveStreams;

				std::string overall;
				Chess::PgnGame pgngame;
				Chess::GameManager game;
				game.InitPgnGame(pgngame);
				while (true)
				{
					if (m_EndThread)
					{
						EngineApp.Write("quit");

						return;
					}

					overall.clear();

					std::string cur_fen = game.GetFen();

					EngineApp.Write("isready");
					
					s_vectorMutex.lock();

					for (auto& command : m_write)
					{
						EngineApp.Write(command);
						int indexFen = command.find("position");
						if (indexFen != std::string::npos)
						{
							cur_fen = std::string(command.begin() + indexFen + 13, command.end());
							overall.clear();
						}
					}

					m_write.clear();
					s_vectorMutex.unlock();

					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					overall += EngineApp.Read();
					
					int indexOff = 0;

					StockMoveStreams.clear();

					while (indexOff != std::string::npos && indexOff < overall.size())
					{
						int indexStart = overall.find("info", indexOff);
						if (indexStart == std::string::npos)
							break;
						int indexEnd = overall.find('\n', indexStart);
						if (indexEnd == std::string::npos)
							break;

						std::string stream = overall.substr(indexStart, indexEnd - indexStart);
						
						if(stream.find("score") != std::string::npos)
							StockMoveStreams.emplace_back(stream);

						indexOff = indexEnd;
					}
					
					m_FinalStreams[0] = -1;
					m_FinalStreams[1] = -1;
					m_FinalStreams[2] = -1;
					m_FinalStreams[3] = -1;
					m_FinalStreams[4] = -1;

					for (int i = 0; i < StockMoveStreams.size(); i++)
					{
						int indexList = StockMoveStreams[i].find("multipv");

						if (indexList == std::string::npos)
						{
							m_FinalStreams[0] = i;
							continue;
						}

						m_FinalStreams[int(StockMoveStreams[i][indexList + 8]) - 49] = i;
					}

					for(int list = 0; list < 5; list++)
					{
						if (s_timeScore.Elapsed() < 0.5)
							break;

						if (m_FinalStreams[list] == -1)
							continue;

						overall = StockMoveStreams[m_FinalStreams[list]];

						int index = FindLastOf(overall, " depth ");
						if (index + 7 < overall.size() && index >= 0 && list == 0)
						{
							index += 7;
							std::string strdepth;
							for (int j = index; j < overall.size(); j++)
							{
								if (overall[j] == ' ')
									break;
								strdepth += overall[j];
							}
							m_Depth = std::stoi(strdepth);
						}

						index = FindLastOf(overall, " cp ");
						if (index + 4 < overall.size() && index >= 0)
						{
							index += 4;
							std::string strscore;
							for (int j = index; j < overall.size(); j++)
							{
								if (overall[j] == ' ')
									break;
								strscore += overall[j];
							}
							m_Score[list] = (float)std::stoi(strscore) / 100.0f;
							if (m_BlackToPlay)
								m_Score[list] *= -1;
						}
						else if (index == -1)
						{
							index = FindLastOf(overall, "mate");
							if (index + 5 < overall.size() && index >= 0)
							{
								index += 5;
								std::string strscore;
								for (int j = index; j < overall.size(); j++)
								{
									if (overall[j] == ' ')
										break;
									strscore += overall[j];
								}
								float score = (float)std::stoi(strscore);
								if(score == 0)
									m_Score[list] = 1000.0f;
								else
									m_Score[list] = score + 1000.0f * std::abs(score) / score;
								if (m_BlackToPlay)
									m_Score[list] *= -1;
							}
						}

						index = FindLastOf(overall, " nps ");
						if (index + 5 < overall.size() && index >= 0 && list == 0)
						{
							index += 5;
							std::string strnps;
							for (int j = index; j < overall.size(); j++)
							{
								if (overall[j] == ' ')
									break;
								strnps += overall[j];
							}
							m_Nps = std::stoi(strnps);
						}

						index = FindLastOf(overall, " pv ");
						if (index + 4 < overall.size() && index >= 0)
						{
							index += 4;
							int MoveIntex = 0;

							std::vector<std::string> helperVector;
							helperVector.emplace_back("");
							for (int j = index; j < overall.size(); j++)
							{
								if (overall[j] == '\n')
									break;
								if (overall[j] == ' ')
								{
									if (helperVector.size() == 15)
										break;
									helperVector.emplace_back("");
									MoveIntex += 1;
									continue;
								}
								helperVector[MoveIntex] += overall[j];
							}

							pgngame.Clear();
							pgngame["FEN"] = cur_fen;
							game.InitPgnGame(pgngame);

							int indexhere = 0;
							for (auto& move : helperVector)
							{
								if (move.empty())
								{
									helperVector.resize(indexhere);
									break;
								}
								if ((int)move.find('\r') + 1)
									move.erase(move.find('\r'));

								glm::vec2 pos(move[0] - 'a', move[1] - '1'), dir(move[2] - 'a', move[3] - '1');
								int n_type = 0;
								if (move.size() == 5)
								{
									std::string n_typeStr = "  nbrq ";
									n_type = n_typeStr.find(move[4]);
									if (n_type == -1)
										n_type = 0;
								}

								move = game.ConvertUCIStringToString(move);
			
								if (game.MakeMove(move) == Chess::Board::MOVEERROR)
								{
									helperVector.resize(indexhere);
									break;
								}

								indexhere++;
							}
							game.GoInitialPosition();
							
							s_moveMutex.lock();
							
							m_Moves[list].clear();
							m_Moves[list] = helperVector;
							
							s_moveMutex.unlock();
						}
					}
				}
			}
		);
	}

	void EnginePanel::CloseEngine()
	{
		if (!IsEngineOpen())
			return;

		g_ChessEngineOpen = false;
		m_EndThread = true;
		m_processThread->join();
		delete m_processThread;
		m_processThread = nullptr;
		m_running = false;
		m_oldBoard.clear();
		Reset();
	}

	std::vector<std::string>& EnginePanel::GetAvailEngines()
	{
		return m_AvailEngines;
	}

	void EnginePanel::CommandChessEngine(const std::string& command)
	{
		s_vectorMutex.lock();
		if (IsEngineOpen())
		{
			m_write.push_back(command);
		}

		if ((int)command.find("position fen") + 1)
		{
			if ((int)command.find('w') == -1)
				m_BlackToPlay = true;
			else
				m_BlackToPlay = false;
		}

		s_vectorMutex.unlock();
	}

	bool EnginePanel::IsEngineOpen() const
	{
		return (bool)m_processThread;
	}

	void EnginePanel::GetBestMoveStr(int list, std::vector<std::string>& moves) const
	{
		moves.clear();

		if (s_timeScore.Elapsed() < 0.5)			
			return;

		s_moveMutex.lock();
		
		moves = m_Moves[list];
		
		s_moveMutex.unlock();
	}

	int EnginePanel::GetDepth() const
	{
		if (IsEngineOpen())
			return m_Depth;
		return 404;
	}

	int EnginePanel::GetNodesPerSecond() const
	{
		if (IsEngineOpen())
			return m_Nps;
		return 404;
	}

	std::string EnginePanel::GetName()
	{
		return m_EngineName;
	}
}
