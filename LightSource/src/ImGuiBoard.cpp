#include "ImGuiBoard.h"

#include "ChessAPI.h"
#include "ChessCore/chess_board.h"
#include "ChessCore/chess_entry.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include <fstream>

#include "../../Walnut/Source/Walnut/Application.h"

static std::string s_fen;

void ImGuiBoard::OnAttach()
{
	m_board[0] = std::make_shared<Walnut::Image>("board.png");
	m_board[1] = std::make_shared<Walnut::Image>("boardRev.png");

	m_pieces[0] = std::make_shared<Walnut::Image>("piecies\\white_pawn.png");
	m_pieces[1] = std::make_shared<Walnut::Image>("piecies\\white_knight.png");
	m_pieces[2] = std::make_shared<Walnut::Image>("piecies\\white_bishop.png");
	m_pieces[3] = std::make_shared<Walnut::Image>("piecies\\white_rook.png");
	m_pieces[4] = std::make_shared<Walnut::Image>("piecies\\white_queen.png");
	m_pieces[5] = std::make_shared<Walnut::Image>("piecies\\white_king.png");
	m_pieces[6] = std::make_shared<Walnut::Image>("piecies\\black_pawn.png");
	m_pieces[7] = std::make_shared<Walnut::Image>("piecies\\black_knight.png");
	m_pieces[8] = std::make_shared<Walnut::Image>("piecies\\black_bishop.png");
	m_pieces[9] = std::make_shared<Walnut::Image>("piecies\\black_rook.png");
	m_pieces[10] = std::make_shared<Walnut::Image>("piecies\\black_queen.png");
	m_pieces[11] = std::make_shared<Walnut::Image>("piecies\\black_king.png");

	m_circleFromStart = std::make_shared<Walnut::Image>("f.png");
	m_circleToEnd = std::make_shared<Walnut::Image>("f2.png");

	m_RedX = std::make_shared<Walnut::Image>("Resources\\RedX.png");

	UpdateBoardValues();

	//ImGui::StyleColorsDark();
}

void ImGuiBoard::OnUIRender()
{
	ImGui::Begin("Hello", 0, ImGuiWindowFlags_NoScrollbar);
	
	if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) && !ImGui::IsAnyMouseDown())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
		{
			m_NextMove = true;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
		{
			ChessAPI::PreviousSavedMove();
		}
	}

	m_size = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 2 * ImGui::GetStyle().ItemSpacing.y;

	m_startCursor.y = ImGui::GetCursorPosY();
	m_startCursor.x = ImGui::GetWindowContentRegionWidth() / 2 - m_size / 2;

	RenderBoard();
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			const wchar_t* path = (const wchar_t*)payload->Data;
			std::wstring wstrpath = path;
			std::string strpath = std::string(wstrpath.begin(), wstrpath.end());
			if (strpath.find(".pgn") == strpath.size() - 5)
			{
				printf("Could not load {0} - not a chess file", path);
			}
			else
				ChessAPI::SetNewChessGame(strpath);
		}
		ImGui::EndDragDropTarget();
	}
	RenderPieces();

	if (m_CapturedPieceIndex>0)
		RenderCirclesAtPossibleMoves();

	auto MousePos = FindMousePos();
	

	//ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
	//ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
	//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });

	ImVec2 bsize = { m_size / 10, m_size / 10 };
	
	//Piece Moving and Playing
	if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (MousePos.x > -1 && MousePos.y > -1
				&& MousePos.x < 8 && MousePos.y < 8)
			{
				m_CapturedPieceIndex = m_block[MousePos.y][MousePos.x];
				m_oldNumX = MousePos.x;
				m_oldNumY = MousePos.y;
				m_block[MousePos.y][MousePos.x] = 0;
			}
		}
		else if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_CapturedPieceIndex)
		{
			ImGui::SetCursorPos(ImVec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x - bsize.x / 2, ImGui::GetMousePos().y - ImGui::GetWindowPos().y - bsize.y / 2));
			ImGui::Image((ImTextureID)m_pieces[m_CapturedPieceIndex - 1]->GetRendererID(), bsize);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_CapturedPieceIndex)
		{
			if (MousePos.x > -1 && MousePos.y > -1
				&& MousePos.x < 8 && MousePos.y < 8)
			{
				m_block[MousePos.y][MousePos.x] = m_CapturedPieceIndex;
				m_CapturedPieceIndex = 0;

				if (glm::vec2{ m_oldNumX, m_oldNumY } != glm::vec2{ MousePos.x , MousePos.y })
				{
					if (!m_reverse)
						ChessAPI::IsMoveValid({ m_oldNumX, 7 - m_oldNumY }, { MousePos.x , 7 - MousePos.y });
					else
						ChessAPI::IsMoveValid({ 7 - m_oldNumX, m_oldNumY }, { 7 - MousePos.x , MousePos.y });
				}

				UpdateBoardValues();
			}
			else
			{
				m_block[m_oldNumY][m_oldNumX] = m_CapturedPieceIndex;
				m_CapturedPieceIndex = 0;
			}
		}
		else
			UpdateBoardValues();
	}
	//ImGui::PopStyleColor(3);

	//check if there are multiple next moves
	if (m_NextMove)
	{
		std::vector<int> movePath = ChessAPI::GetMoveIntFormat();
	
		chess::Pgn_Game::ChessMovesPath curMovesRef;
		ChessAPI::GetMovesPgnFormat(curMovesRef);
		chess::Pgn_Game::ChessMovesPath* curMoves = &curMovesRef;
	
		for (int i = 1; i < movePath.size(); i++)
			if (i % 2 == 1)
				curMoves = &curMoves->children[movePath[i]];
	
		int index = -1;
		for (int i = movePath[movePath.size() - 1] + 1; i < curMoves->move.size(); i++)
			if (curMoves->move[i] != "child")
			{
				index = i;
				break;
			}
	
		bool openPopup = false;
		if (index + 1)
		{
			for (int i = index + 1; i < curMoves->move.size(); i++)
			{
				if (curMoves->move[i] != "child")
				{
					if (i - index > 1)
						openPopup = true;
					break;
				}
				if (i + 1 == curMoves->move.size())
					openPopup = true;
	
			}
		}
	
		if (openPopup)
		{
			ImGui::OpenPopup("Move_Choose");
	
			m_PossibleNextMoves.clear();
			
			int amountOfPreviousChildren = 0;
			for (int i = 0; i < index; i++)
				if (curMoves->move[i] == "child")
					amountOfPreviousChildren += 1;
	
			int indexChild = 0;
			for (int i = index + 1; i < curMoves->move.size(); i++)
			{
				if (curMoves->move[i] == "child")
				{
					auto curPath = movePath;
					curPath[curPath.size() - 1] = i;
					curPath.push_back(amountOfPreviousChildren + indexChild);
					curPath.push_back(0);
	
					m_PossibleNextMoves[curMoves->children[amountOfPreviousChildren + indexChild].move[0]] = curPath;
	
					indexChild += 1;
				}
				else
					break;
			}
	
			m_MainMove = curMoves->move[index];
		}
		else
			ChessAPI::NextSavedMove();
		m_NextMove = false;
	}

	//check if there multible variations
	if (ChessAPI::IsNewVariationAdded())
	{
		ImGui::OpenPopup("New_Variant");
	}

	//check if a pawn is ready to be promoted
	if (ChessAPI::IsWaitingForNewType() && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		ImGui::OpenPopup("New_Piece");

	m_Center = ImGui::GetWindowPos();
	m_Center.x += ImGui::GetContentRegionMax().x;
	m_Center.y += ImGui::GetContentRegionMax().y * 0.25;

	//Open Editor
	if (m_ToOpenEditor)
	{
		m_ToOpenEditor = false;
		ImGui::OpenPopup("Editor");
	}

	NextMovePopup();
	NewVariantPopup();
	NewPiecePopup();
	EditorPopup();

	ImGui::End();

}

void ImGuiBoard::RenderBoard()
{
	ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionWidth() / 2 - m_size / 2, m_startCursor.y));

	auto& board = m_board[m_reverse];
	ImGui::Image((ImTextureID)board->GetRendererID(), { m_size, m_size });
}

void ImGuiBoard::RenderPieces()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float blockSize = m_size / 9;

	float xposition = blockSize;
	float yposition = blockSize;
	
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (m_block[i][j])
			{
				ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + m_startCursor.x, yposition - bsize.y / 2 + m_startCursor.y));
				ImGui::Image((ImTextureID)m_pieces[m_block[i][j] - 1]->GetRendererID(), bsize);
			}

			if (i == m_oldNumY && j == m_oldNumX && m_CapturedPieceIndex != 0)
			{
				ImGui::SetCursorPos(
					ImVec2(xposition - bsize.x / 2 + m_startCursor.x + bsize.x / 2.5f / 2.0f + bsize.x/2.0f - bsize.x / 2.5f
						, yposition - bsize.y / 2 + m_startCursor.y + bsize.y / 2.5f / 2.0f + bsize.y / 2.0f - bsize.y / 2.5f));
				ImGui::Image((ImTextureID)m_circleFromStart->GetRendererID(), { bsize.x / 2.5f, bsize.y / 2.5f });
			}

			xposition += blockSize;
		}
		xposition = blockSize;
		yposition += blockSize;
	}
}

void ImGuiBoard::RenderCirclesAtPossibleMoves()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float blockSize = m_size / 9;

	float xposition = blockSize;
	float yposition = blockSize;

	std::vector<vec2<float>> possibleMoves;
	if (!m_reverse)
		possibleMoves = ChessAPI::GetPossibleDirections({ m_oldNumX, 7 - m_oldNumY });
	else
		possibleMoves = ChessAPI::GetPossibleDirections({ 7 - m_oldNumX, m_oldNumY });
	for (auto& move : possibleMoves)
	{
		if (!m_reverse)
		{
			xposition = (move.x() + m_oldNumX + 1) * blockSize;
			yposition = (9 -(move.y() +7- m_oldNumY + 1)) * blockSize;
		}
		else
		{
			xposition = (9-(move.x() +7 - m_oldNumX + 1)) * blockSize;
			yposition = (move.y() + m_oldNumY + 1) * blockSize;
		}

		ImGui::SetCursorPos(
			ImVec2(xposition - bsize.x / 2 + m_startCursor.x
				, yposition - bsize.y / 2 + m_startCursor.y));
		ImGui::Image((ImTextureID)m_circleToEnd->GetRendererID(), { bsize.x, bsize.y });
	}
}

ImVec2 ImGuiBoard::FindMousePos()
{
	float blockSize = m_size / 9;

	static int numY = 0;
	static int numX = 0;

	numX = (ImGui::GetMousePos().x - m_startCursor.x - ImGui::GetWindowPos().x - blockSize / 2) / blockSize;
	if (ImGui::GetMousePos().x - m_startCursor.x - ImGui::GetWindowPos().x - blockSize / 2 < 0)
		numX = -1;
	numY = (ImGui::GetMousePos().y - m_startCursor.y - ImGui::GetWindowPos().y - blockSize / 2) / blockSize;
	if (ImGui::GetMousePos().y - m_startCursor.y - ImGui::GetWindowPos().y - blockSize / 2 < 0)
		numY = -1;

	return ImVec2(numX, numY);
}

void ImGuiBoard::UpdateBoardValues()
{
	if (!m_reverse)
	{
		int indexID = 0;
		for (int i = 7; i > -1; i--)
		{
			for (int j = 0; j < 8; j++)
			{
				m_block[i][j] = ChessAPI::GetBlockID(indexID);
				indexID++;
			}
		}
	}
	else
	{
		int indexID = 0;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 7; j > -1; j--)
			{
				m_block[i][j] = ChessAPI::GetBlockID(indexID);
				indexID++;
			}
		}
	}
}

void ImGuiBoard::NextMovePopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing, ImVec2(1, 1));
	if (ImGui::BeginPopupModal("Move_Choose", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::PushFont(Walnut::Application::GetFont("Bold"));
		if (ImGui::Selectable(m_MainMove.c_str()) ||
			(ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			ChessAPI::NextSavedMove();
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopFont();

		for (auto it = m_PossibleNextMoves.begin(); it != m_PossibleNextMoves.end(); it++)
		{
			if (ImGui::Selectable(it->first.c_str()) ||
				(ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
			{
				ChessAPI::GoMoveByIntFormat(it->second);
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Play Main"))
		{
			ChessAPI::NextSavedMove();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cansel"))
			ImGui::CloseCurrentPopup();
		if (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

}

void ImGuiBoard::NewVariantPopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing, ImVec2(1, 1));
	if (ImGui::BeginPopupModal("New_Variant", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::Selectable("New Variation");

		if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow))
			ImGui::CloseCurrentPopup();

		if (ImGui::Selectable("Promote to MainLine")
			|| (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			auto movePathToPromote = ChessAPI::GetMoveIntFormat();
			ChessAPI::PreviousSavedMove();
			ChessAPI::NextSavedMove();
			auto movePathToGo = ChessAPI::GetMoveIntFormat();

			ChessAPI::PromoteVariation(movePathToPromote);
			ChessAPI::GoMoveByIntFormat(movePathToGo);
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable("OverWrite")
			|| (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			auto movePath = ChessAPI::GetMoveIntFormat();
			ChessAPI::PreviousSavedMove();
			ChessAPI::NextSavedMove();
			auto movePathToGo = ChessAPI::GetMoveIntFormat();

			ChessAPI::PromoteVariation(movePath);
			ChessAPI::DeleteVariation(movePath);
			ChessAPI::GoMoveByIntFormat(movePathToGo);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Play Default"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cansel")
			|| (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)))
		{
			auto movePath = ChessAPI::GetMoveIntFormat();
			ChessAPI::PreviousSavedMove();
			auto movePathToGo = ChessAPI::GetMoveIntFormat();

			ChessAPI::DeleteVariation(movePath);
			ChessAPI::GoMoveByIntFormat(movePathToGo);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ImGuiBoard::NewPiecePopup()
{
	if (ImGui::BeginPopup("New_Piece", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		if (!ChessAPI::IsWaitingForNewType())
			ImGui::CloseCurrentPopup();

		bool color = ChessAPI::GetPlayerColor();
		int index = color ? 1 : 0;

		for (int i = 0; i < 4; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[10-i - index * 6]->GetRendererID(), { 100, 100 }))
			{
				ChessAPI::SetNewPieceType(5 - i);
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
		{
			ChessAPI::PreviousSavedMove();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ImGuiBoard::EditorPopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Editor"))
	{
		ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		ImVec2 ViewportSize = { viewportPanelSize.x, viewportPanelSize.x };
		
		auto editorSize = ImGui::GetWindowWidth() - ImGui::GetCursorPosX() -6 * ImGui::GetStyle().ItemSpacing.x;

		ImVec2 bsize = { editorSize / 10, editorSize / 10 };
		float blockSize = editorSize / 9;
		
		ImVec2 editorStartCursor;
		editorStartCursor.y = ImGui::GetCursorPosY();
		editorStartCursor.x = ImGui::GetWindowContentRegionWidth() / 2 - editorSize / 2;

		//render board
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionWidth() / 2 - editorSize / 2, editorStartCursor.y));
		ImGui::Image((uint32_t*)m_board[0]->GetRendererID(), { editorSize, editorSize });

		auto cursorEnd = ImGui::GetCursorPos();

		//render Pieces

		float xposition = blockSize;
		float yposition = blockSize;

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (m_Editorblock[i][j])
				{
					ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + editorStartCursor.x, yposition - bsize.y / 2 + editorStartCursor.y));
					ImGui::Image((ImTextureID)m_pieces[m_Editorblock[i][j] - 1]->GetRendererID(), bsize);
				}

				xposition += blockSize;
			}
			xposition = blockSize;
			yposition += blockSize;
		}

		static int pointIndex = -1;

		ImGui::SetCursorPos(cursorEnd);

		for (int i = 0; i < 6; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[i]->GetRendererID(), { 50, 50 }))
			{
				pointIndex = i;
			}
			ImGui::SameLine();
		}

		ImGui::NewLine();
		for (int i = 6; i < 12; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[i]->GetRendererID(), { 50, 50 }))
			{
				pointIndex = i;
			}
			ImGui::SameLine();
		}
		if (ImGui::ImageButton((uint32_t*)m_RedX->GetRendererID(), { 50, 50 }, { 0, 1 }, { 1, 0 }))
		{
			pointIndex = -1;
		}

		cursorEnd = ImGui::GetCursorPos();

		ImGui::SetCursorPos(ImVec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x - bsize.x / 2, ImGui::GetMousePos().y - ImGui::GetWindowPos().y - bsize.y / 2));
		if (ImGui::GetCursorPos().y > cursorEnd.y - 50)
			ImGui::SetCursorPosY(cursorEnd.y - 50);

		if (pointIndex == -1)
		{
			ImGui::Image((ImTextureID)m_RedX->GetRendererID(), bsize);
		}
		else
		{
			ImGui::Image((ImTextureID)m_pieces[pointIndex]->GetRendererID(), bsize);
		}

		//mousePos
		static int numY = 0;
		static int numX = 0;

		numX = (ImGui::GetMousePos().x - editorStartCursor.x - ImGui::GetWindowPos().x - blockSize / 2) / blockSize;
		if (ImGui::GetMousePos().x - editorStartCursor.x - ImGui::GetWindowPos().x - blockSize / 2 < 0)
			numX = -1;
		numY = (ImGui::GetMousePos().y - editorStartCursor.y - ImGui::GetWindowPos().y - blockSize / 2) / blockSize;
		if (ImGui::GetMousePos().y - editorStartCursor.y - ImGui::GetWindowPos().y - blockSize / 2 < 0)
			numY = -1;

		ImVec2 MousePos = { (float)numX, (float)numY };
		
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (MousePos.x > -1 && MousePos.y > -1
				&& MousePos.x < 8 && MousePos.y < 8)
			{
				if (pointIndex + 1 == m_Editorblock[MousePos.y][MousePos.x])
					m_Editorblock[MousePos.y][MousePos.x] = 0;
				else
					m_Editorblock[MousePos.y][MousePos.x] = pointIndex + 1;
			}
		}

		ImGui::SetCursorPos(cursorEnd);

		//ImGui::SeparatorText("Settings");
		ImGui::Separator();
		ImGui::Text("Settings:");

		ImGui::Columns(2);

		static bool w_BigRoke = false, b_BigRoke = false, w_SmallRoke = false, b_SmallRoke = false;

		ImGui::PushID("W");
		ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.2, 0.2, 0.2, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8, 0.8, 0.8, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.8, 0.8, 0.8, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.8, 0.8, 0.8, 1));
		ImGui::Checkbox("0-0-0", &w_BigRoke);
		ImGui::SameLine();
		ImGui::Checkbox("0-0", &w_SmallRoke);
		ImGui::PopStyleColor(4);
		ImGui::PopID();

		ImGui::PushID("B");
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25, 0.25, 0.25, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25, 0.25, 0.25, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25, 0.25, 0.25, 1));
		ImGui::Checkbox("0-0-0", &b_BigRoke);
		ImGui::SameLine();
		ImGui::Checkbox("0-0", &b_SmallRoke);
		ImGui::PopStyleColor(3);
		ImGui::PopID();

		ImGui::NextColumn();

		static int player = 1;
		ImGui::RadioButton("White", &player, 1);
		ImGui::SameLine();
		ImGui::RadioButton("Black", &player, 0);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
		if (ImGui::Button("New"))
		{
			chess::Pgn_Game gamePgn;
			chess::chess_entry gameNew(gamePgn);
			gameNew.run();
			
			int indexID = 0;
			for (int i = 7; i > -1; i--)
			{
				for (int j = 0; j < 8; j++)
				{
					m_Editorblock[i][j] = gameNew.get_blockID(vec2<float>(indexID - 8 * (int)(indexID / 8), (int)(indexID / 8)));
					indexID++;
				}
			}
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
		if (ImGui::Button("Clear"))
		{
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					m_Editorblock[i][j] = 0;
				}
			}
		}
		ImGui::PopStyleColor();

		ImGui::Columns();

		ImGui::Separator();
		ImGui::NewLine();

		auto CheckBoard = [this](std::string& fen)
			{
				std::string values = " PNBRQKpnbrqk";
				int boardValue[64];

				int indexBoard = 63;
				for (int i = 0; i < 8; i++)
				{
					for (int j = 7; j > -1; j--)
					{
						boardValue[indexBoard] = m_Editorblock[i][j];
						indexBoard--;
					}
				}

				fen = "";
				int empty = 0;
				bool once = false;

				//den paizei na exw graphei pio epikinduni function
				for (int i = 56; i > -1; i++)
				{
					if (i % 8 == 0 && once)
					{
						once = false;
						if (empty)
						{
							fen += std::to_string(empty);
							empty = 0;
						}
						if (i != 8)
							fen += '/';
						i -= 17;
						continue;
					}
					once = true;
					if (boardValue[i] == 0)
					{
						empty += 1;
						continue;
					}
					if (empty)
					{
						fen += std::to_string(empty);
						empty = 0;
					}
					fen += values[boardValue[i]];
				}

				fen += ' ';

				if (player == 1)
					fen += 'w';
				else
					fen += 'b';

				fen += ' ';

				if (w_SmallRoke)
					fen += 'K';
				if (w_BigRoke)
					fen += 'Q';
				if (b_SmallRoke)
					fen += 'k';
				if (b_BigRoke)
					fen += 'q';

				if (fen[fen.size() - 1] == ' ')
					fen += '-';

				fen += ' ';
				fen += '-';
				fen += ' ';
				fen += '0';
				fen += ' ';
				fen += '1';

				chess_core::board edGame;
				edGame.set_position(fen);

				return !edGame.ERROR_EXIT;
			};


		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
		if (ImGui::Button("OverWrite"))
		{
			std::string fen;
			if (CheckBoard(fen))
			{
				auto& PgnGame = *ChessAPI::GetPgnGame();
				PgnGame.clear();
				PgnGame["FEN"] = fen;
				ChessAPI::OverWriteChessFile("");

				ImGui::CloseCurrentPopup();
				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			else
			{
				ImGui::OpenPopup("Error");
			}

		}
		ImGui::SameLine();
		if (ImGui::Button("Create As New Game"))
		{
			std::string fen;
			if (CheckBoard(fen))
			{
				ChessAPI::NewGameInFile();
				auto& PgnGame = *ChessAPI::GetPgnGame();
				PgnGame.clear();
				PgnGame["FEN"] = fen;
				ChessAPI::OverWriteChessFile("");

				ImGui::CloseCurrentPopup();
				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			else
			{
				ImGui::OpenPopup("Error");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Create As New File"))
		{
			s_fen.clear();
			if (CheckBoard(s_fen))
			{
				ImGui::OpenPopup("New Chess File");

				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			else
			{
				ImGui::OpenPopup("Error");
			}
		}
		ImGui::PopStyleColor();

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Error", 0, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrapped("Invalid Board!");

			ImGui::NewLine();

			ImGui::PushID("in");

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Close").x - 8);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();

			ImGui::PopID();

			ImGui::EndPopup();
		}

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New Chess File", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			static std::string s_inputNName = "NewFile.pgn";
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			if (ImGui::Button("Create"))
			{
				std::filesystem::path nPath = std::filesystem::path()/ "chess_working_directory" / s_inputNName;
				nPath.replace_extension(".pgn");

				std::string strNPath = nPath.string();
					
				ChessAPI::OverWriteChessFile("");

				chess::Pgn_File NPgnFile;
				NPgnFile.CreateGame();
				NPgnFile[0]["FEN"] = s_fen;
				std::ofstream outfile(strNPath);
				outfile << NPgnFile;
				outfile.close();

				ChessAPI::SetNewChessGame(strNPath);

				ImGui::ClosePopupToLevel(0, true);
				//ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Cansel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		ImGui::SameLine();

		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Cansel").x - 13);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
		if (ImGui::Button("Cansel"))
		{
			ImGui::CloseCurrentPopup();
			ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}
}

void ImGuiBoard::FlipBoard()
{
	m_reverse = !m_reverse;
}

void ImGuiBoard::OpenEditor()
{
	m_ToOpenEditor = true;

	int indexID = 0;
	for (int i = 7; i > -1; i--)
	{
		for (int j = 0; j < 8; j++)
		{
			m_Editorblock[i][j] = ChessAPI::GetBlockID(indexID);
			indexID++;
		}
	}
}