#include "ImGuiBoard.h"

#include "ChessAPI.h"

#include "../../Walnut/Source/Walnut/Application.h"


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

	UpdateBoardValues();

	//ImGui::StyleColorsDark();
}

void ImGuiBoard::OnUIRender()
{
	ImGui::Begin("Hello", 0, ImGuiWindowFlags_NoScrollbar);
	
	if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
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

	//flip
	if (ImGui::IsKeyPressed(ImGuiKey_F) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		m_reverse = !m_reverse;

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

	NextMovePopup();
	NewVariantPopup();
	NewPiecePopup();

	ImGui::End();
}

void ImGuiBoard::RenderBoard()
{
	ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionWidth() / 2 - m_size / 2, ImGui::GetCursorPosY()));

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
			xposition += blockSize;
		}
		xposition = blockSize;
		yposition += blockSize;
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

void ImGuiBoard::FlipBoard()
{
	m_reverse = !m_reverse;
}