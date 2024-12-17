#include "GameManager.h"

static std::string s_strMoveTypes = " NBRQK";
static std::string s_strMoveTypesSmall = " nbrqk";
static std::string s_strMoveIndexX = "abcdefgh";
static std::string s_strMoveIndexY = "12345678";

namespace Chess
{
	void GameManager::InitPgnGame(Pgn_Game& pgnGame)
	{
		m_pgnGame = &pgnGame;

		m_Board.NewPosition((*m_pgnGame)["FEN"]);

		m_mapMoves.clear();
		m_lastMoveKey.clear();

		m_lastMoveKey.emplace_back(-1);
	}

	void GameManager::GetFen(std::string& fen) const
	{
		fen = m_Board.GetFen();
	}

	std::string GameManager::GetFen() const
	{
		return m_Board.GetFen();
	}

	std::vector<uint8_t> GameManager::GetFormatedFEN() const
	{
		return m_Board.GetFormatedFen();
	}

	Color GameManager::GetPlayerToPlay() const
	{
		return m_Board.GetPlayerToPlayColor();
	}

	GameManager::PieceID GameManager::GetPieceID(int index) const
	{
		return { m_Board.GetPieceType(index), m_Board.GetPieceColor(index) };
	}

	GameManager::PieceID GameManager::GetPieceID(int indexX, int indexY) const
	{
		return { m_Board.GetPieceType(indexX, indexY), m_Board.GetPieceColor(indexX, indexY) };
	}

	std::string& GameManager::GetNote(const MoveKey& moveKey)
	{
		if (moveKey.size() == 1 && moveKey[0] == -1)
			return m_pgnGame->GetFirstNote();
			
		auto currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < moveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[moveKey[i]];
		}

		return currentPath->details[moveKey[moveKey.size() - 1]];
	}

	Pgn_Game::ChessMovesPath GameManager::GetMovesByStr() const
	{
		if(m_pgnGame)
			return (*m_pgnGame).GetMovePathbyCopy();
		return {};
	}

	GameManager::MoveKey GameManager::GetLastMoveKey() const
	{
		return m_lastMoveKey;
	}

	void GameManager::GetAvailableMoves(std::vector<Board::Move>& moves) const
	{
		m_Board.GetAvailableMoves(moves);
	}

	bool GameManager::IsMoveValid(Board::Move move) const
	{
		return m_Board.IsMoveValid(move);
	}

	Board::MakeMoveStatus GameManager::MakeMove(Board::Move move, Piece	promotedType)
	{
		if (!m_Board.IsMoveValid(move))
			return Board::ERROR;

		MoveData moveD;
		ConvertMoveToMoveData(move, moveD, promotedType);

		if(moveD.pieceToMove == NONE)
			return Board::ERROR;

		std::string strmove;
		ConvertMoveDataToString(moveD, strmove);

		auto ret = m_Board.MakeMove(move, promotedType);

		if(ret == Board::SUCCESS)
			AddMove(strmove, moveD);

		return ret;
	}

	Board::MakeMoveStatus GameManager::MakeMove(const std::string& move)
	{
		if(move.empty())
			return Board::ERROR;

		MoveData moveD;
		ConvertStringToMoveData(move, moveD);

		if (!m_Board.IsMoveValid(moveD.move) || moveD.pieceToMove == NONE)
			return Board::ERROR;
		
		auto ret = m_Board.MakeMove(moveD.move, moveD.piecePromote);

		if(ret == Board::SUCCESS)
			AddMove(move, moveD);

		return ret;
	}

	void GameManager::GoNextMove()
	{
		Pgn_Game::ChessMovesPath* currentPath = nullptr;
		GetCurrentPgnMovePath(currentPath);

		bool nextExists = false;

		int nextIndex = -1;

		for (int i = m_lastMoveKey[m_lastMoveKey.size() - 1] + 1; i < currentPath->move.size(); i++)
		{
			if (currentPath->move[i] != "child")
			{
				nextExists = true;
				nextIndex = i;
				break;
			}
		}

		if (!nextExists)
			return;
		
		int oldValue = m_lastMoveKey[m_lastMoveKey.size() - 1];

		m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex;

		if (!m_mapMoves.contains(m_lastMoveKey))
		{
			MoveData moveD;
			ConvertStringToMoveData(currentPath->move[nextIndex], moveD);
			
			if (m_Board.MakeMove(moveD.move, moveD.piecePromote) != Board::SUCCESS)
			{
				m_lastMoveKey[m_lastMoveKey.size() - 1] = oldValue;
				return;
			}
			
			m_mapMoves[m_lastMoveKey] = moveD;
		}
		else
			m_Board.MakeMove(m_mapMoves[m_lastMoveKey].move, m_mapMoves[m_lastMoveKey].piecePromote);
	}

	void GameManager::GoPreviusMove()
	{
		if (!m_mapMoves.contains(m_lastMoveKey))
			return;//Something Bad Happened here

		MoveData moveD = m_mapMoves[m_lastMoveKey];

		if(m_Board.GetPlayerToPlayColor() == WHITE)
			m_Board.SetBlackMovesCount(m_Board.GetBlackMovesCount() - 1);

		m_Board.SetFiftyMoveCount(moveD.FiftyMoveCounter);
		m_Board.SetLastMoveIndex(moveD.lastMoveIndex);

		m_Board.SetKRoke(moveD.K);
		m_Board.SetQRoke(moveD.Q);
		m_Board.SetkRoke(moveD.k);
		m_Board.SetqRoke(moveD.q);
		
		m_Board.RemovePiece(moveD.move.index + moveD.move.move);

		if (moveD.pieceToMove == PAWN && moveD.move.index + moveD.move.move == moveD.lastMoveIndex)
			m_Board.AddPiece(PAWN, m_Board.GetPlayerToPlayColor(), moveD.move.index + moveD.move.move + m_Board.GetPlayerToPlayColor() == BLACK ? -8 : +8);
		else if (moveD.pieceOnDirection != NONE)
			m_Board.AddPiece(moveD.pieceOnDirection, m_Board.GetPlayerToPlayColor(), moveD.move.index + moveD.move.move);

		m_Board.SwapPlayerToPlay();

		m_Board.AddPiece(moveD.pieceToMove, m_Board.GetPlayerToPlayColor(), moveD.move.index);

		if (moveD.pieceToMove == KING && std::abs(moveD.move.move) == 2)
		{
			m_Board.RemovePiece(moveD.move.index + moveD.move.move/2);
			m_Board.AddPiece(ROOK, m_Board.GetPlayerToPlayColor(), moveD.move.index + (moveD.move.move > 0 ? +3 : -4));
		}

		Pgn_Game::ChessMovesPath* currentPath = nullptr;
		GetCurrentPgnMovePath(currentPath);

		do
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1]--;

			if (m_lastMoveKey[0] == -1)
				return;
			if (m_lastMoveKey[m_lastMoveKey.size() - 1] == -1)
				break;

		} while (currentPath->move[m_lastMoveKey[m_lastMoveKey.size() - 1]] == "child");

		if (m_lastMoveKey[m_lastMoveKey.size() - 1] != -1)
			return;

		m_lastMoveKey.erase(m_lastMoveKey.end() - 1);
		m_lastMoveKey.erase(m_lastMoveKey.end() - 1);

		GetCurrentPgnMovePath(currentPath);

		do
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1]--;

		} while (currentPath->move[m_lastMoveKey[m_lastMoveKey.size() - 1]] == "child");

		do
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1]--;

		} while (currentPath->move[m_lastMoveKey[m_lastMoveKey.size() - 1]] == "child");
	}

	void GameManager::GoInitialPosition()
	{
		while (m_lastMoveKey[0] != -1)
			GoPreviusMove();
	}

	void GameManager::GoToPositionByKey(const MoveKey& moveKey)
	{
		GoInitialPosition();
		
		Pgn_Game::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 0; i < moveKey.size(); i++)
		{
			if (i % 2 == 1)
			{
				GoPreviusMove();
				currentPath = &currentPath->children[moveKey[i]];
			}
			else
			{
				for (int j = 0; j < moveKey[i] + 1; j++)
				{
					if (currentPath->move[j] != "child")
					{
						if (MakeMove(currentPath->move[j]) != Board::SUCCESS)
							return;
					}
				}
			}
		}
	}

	void GameManager::EditVariation(const MoveKey& moveKey, VariationEdit editType)
	{
		auto oldKey = m_lastMoveKey;
		GoInitialPosition();
		m_mapMoves.clear();

		Pgn_Game::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < moveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[moveKey[i]];
		}

		switch (editType)
		{
		case Chess::GameManager::SWAP:
		{
			if (moveKey.size() == 1)//has parent
				break;

			std::unordered_map<int, std::string> detailsNew;
			std::vector<std::string> moveNew;
			std::vector<Pgn_Game::ChessMovesPath> childrenNew;

			int amountOfChildrenInside = 0;
			int oldParentChildrenStay = 0;
			int oldParentStart = 0;
			int oldParentNextMove = -1;

			for (int i = moveKey[moveKey.size() - 3] - 1; i > 0; --i)
			{
				if (currentPath->parent->move[i] != "child")
				{
					oldParentStart = i;
					break;
				}
			}

			for (int i = oldParentStart + 1; i < currentPath->parent->move.size(); ++i)
			{
				bool child = currentPath->parent->move[i] == "child";

				if (child)
					amountOfChildrenInside++;
				else if (oldParentNextMove == -1)
					oldParentNextMove = i;

				if (child && oldParentNextMove == -1)
					oldParentChildrenStay++;
			}
			
			moveNew.emplace_back(currentPath->parent->move[oldParentStart]);
			if (currentPath->parent->details.contains(oldParentStart))
			{
				detailsNew[0] = currentPath->parent->details[oldParentStart];
				currentPath->parent->details.erase(oldParentStart);
			}
			for (int i = oldParentNextMove; i < currentPath->parent->move.size(); ++i)
			{
				moveNew.emplace_back(currentPath->parent->move[i]);
				if (currentPath->parent->details.contains(i))
				{
					detailsNew[i - oldParentNextMove + 1] = currentPath->parent->details[i];
					currentPath->parent->details.erase(i);
				}
			}

			if(oldParentNextMove != -1)
				currentPath->parent->move.resize(oldParentNextMove);
			currentPath->parent->move[oldParentStart] = currentPath->move[0];

			if (currentPath->details.contains(0))
				currentPath->parent->details[oldParentStart] = currentPath->details[0];

			for (int i = 1; i < currentPath->move.size(); ++i)
			{
				currentPath->parent->move.emplace_back(currentPath->move[i]);
				if (currentPath->details.contains(i))
					currentPath->parent->details[currentPath->parent->move.size() - 1] = currentPath->details[i];
			}

			for (int i = currentPath->parent->children.size() - amountOfChildrenInside + oldParentChildrenStay; 
					 i < currentPath->parent->children.size(); ++i)
			{
				childrenNew.emplace_back(currentPath->parent->children[i]);
			}

			currentPath->parent->children.resize(currentPath->parent->children.size() - amountOfChildrenInside + oldParentChildrenStay);

			for (auto& c : currentPath->children)
				currentPath->parent->children.emplace_back(c);

			currentPath->children = childrenNew;
			currentPath->details = detailsNew;
			currentPath->move = moveNew;

			m_pgnGame->GetMovePathbyRef().ReloadChildren();
			m_pgnGame->GetMovePathbyRef().parent = nullptr;			

			break;
		}
		case Chess::GameManager::SWAP_MAIN:
		{

			break;
		}
		case Chess::GameManager::INSIDE_UP:
		{

			break;
		}
		case Chess::GameManager::INSIDE_DOWN:
		{

			break;
		}
		case Chess::GameManager::INSIDE_MAX:
		{

			break;
		}
		case Chess::GameManager::INSIDE_MIN:
		{

			break;
		}
		default:
			break;
		}

		//Go back to our original moveKey;
	}

	void GameManager::DeleteMove(const MoveKey& moveKey)
	{
		GoInitialPosition();

		Pgn_Game::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < moveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[moveKey[i]];
		}

		if (currentPath->move.size() > moveKey[moveKey.size() - 1] + 1
			&& currentPath->move[moveKey[moveKey.size() - 1] + 1] == "child")
		{
			int chindIndex = 0;
			auto moveKeyNew = moveKey;

			for (int i = 0; i < moveKey[moveKey.size() - 1]; ++i)
			{
				if (currentPath->move[i] == "child")
					chindIndex++;
			}

			moveKeyNew[moveKeyNew.size() - 1]++;
			moveKeyNew.emplace_back(chindIndex);
			moveKeyNew.emplace_back(0);

			EditVariation(moveKeyNew, SWAP);
			DeleteMove(moveKeyNew);

			return;
		}

		currentPath->move.resize(moveKey[moveKey.size() - 1]);

		if (currentPath->move.empty() && currentPath->parent)
		{
			auto Parent = currentPath->parent;
			Parent->children.erase(Parent->children.begin() + moveKey[moveKey.size() - 2]);
			Parent->move.erase(Parent->move.begin() + moveKey[moveKey.size() - 3]);
		}

		m_mapMoves.clear();
	}

	std::string GameManager::ConvertUCIStringToString(const std::string& uciMove) const
	{
		std::string output = "";
		MoveData moveD;
		Board::Move move;
		Piece piecePromote = NONE;

		move.index = s_strMoveIndexX.find(uciMove[0]) + 8 * s_strMoveIndexY.find(uciMove[1]);
		move.move = s_strMoveIndexX.find(uciMove[2]) + 8 * s_strMoveIndexY.find(uciMove[3]) - move.index;
		
		if (uciMove.size() > 4)
			piecePromote = (Piece)s_strMoveTypesSmall.find(uciMove[4]);

		ConvertMoveToMoveData(move, moveD, piecePromote);
		ConvertMoveDataToString(moveD, output);

		return output;
	}

	void GameManager::ConvertMoveDataToString(const MoveData& move, std::string& strmove) const
	{
		if (m_Board.GetPlayerToPlayColor() == WHITE) 
			strmove += (std::to_string(m_Board.GetBlackMovesCount()) + ". ");
		if (move.pieceToMove == KING && std::abs(move.move.move) == 2.0f)
		{
			//roke
			if (move.move.move > 0)
				strmove += "O-O";
			else
				strmove += "O-O-O";
		}
		else
		{
			if (move.pieceToMove != PAWN)
				strmove += s_strMoveTypes[(int)move.pieceToMove];
			if (move.pieceToMove == PAWN && move.pieceOnDirection != NONE)
				strmove += s_strMoveIndexX[move.move.index % 8];

			std::vector<int> possibleIndex;
			std::vector<Board::Move> possibleMoves;
			m_Board.GetAvailableMoves(possibleMoves, move.pieceToMove);

			for (auto& m : possibleMoves)
			{
				if ((m.index + m.move) == (move.move.index + move.move.move) && m.index != move.move.index)
					possibleIndex.emplace_back(m.index);
			}

			bool showIndexX = false, showIndexY = false;

			if (!possibleIndex.empty())
			{
				for (auto& pi : possibleIndex)
				{
					if (pi % 8 == move.move.index % 8)
						showIndexY = true;
					if (pi / 8 == move.move.index / 8)
						showIndexX = true;
				}

				if (!showIndexX && !showIndexY)
					showIndexX = true;
			}

			if (showIndexX)
				strmove += s_strMoveIndexX[move.move.index % 8];
			if (showIndexY)
				strmove += s_strMoveIndexY[move.move.index / 8];
			
			if (move.pieceOnDirection != NONE) 
				strmove += 'x';
			
			strmove += s_strMoveIndexX[(move.move.index + move.move.move) % 8];
			strmove += s_strMoveIndexY[(move.move.index + move.move.move) / 8];
		}
		if (move.piecePromote != NONE)
		{
			strmove += '=';
			strmove += s_strMoveTypes[(int)move.piecePromote];
		}

		auto kingSecurity = m_Board.GetKingStatus(m_Board.GetPlayerToPlayColor() == WHITE ? BLACK : WHITE);

		if (kingSecurity == Board::CHECKED)
			strmove += '+';
		else if(kingSecurity == Board::CHECKED)
			strmove += '#';

		if (move.pieceToMove == NONE)
			strmove = "";
	}

	void GameManager::ConvertStringToMoveData(const std::string& strmove, MoveData& move) const
	{
		int startIndex = strmove.find(' ') + 1;

		Piece typeToMove;
		Piece typeToPromote = NONE;

		move.FiftyMoveCounter = m_Board.GetFiftyMoveCount();
		move.lastMoveIndex = m_Board.GetLastMoveIndex();

		move.K = m_Board.GetKRoke();
		move.Q = m_Board.GetQRoke();
		move.k = m_Board.GetkRoke();
		move.q = m_Board.GetqRoke();

		if (strmove[startIndex] == '0' || strmove[startIndex] == 'O')
		{
			/*Roke*/
			if (strmove.find("0-0-0") != std::string::npos || strmove.find("O-O-O") != std::string::npos)
				move.move.move = -2;
			else
				move.move.move = 2;

			if (m_Board.GetPlayerToPlayColor() == WHITE)
				move.move.index = 4;
			else
				move.move.index = 60;

			move.pieceToMove = KING;
			move.pieceOnDirection = NONE;
			move.piecePromote = NONE;

			return;
		}
		else if (strmove[startIndex] >= 'a')
			typeToMove = PAWN;
		else
			typeToMove = (Piece)s_strMoveTypes.find(strmove[startIndex]);

		int strMoveDirectionIndex = startIndex;

		while (strmove[strMoveDirectionIndex] > '8' || (strmove[strMoveDirectionIndex - 1] >= 'A'
			&& strmove.size() - (strMoveDirectionIndex + 1) && strmove[strMoveDirectionIndex + 1] >= 'a'
			&& typeToMove != PAWN))
		{
			strMoveDirectionIndex += 1;
		}

		int directionIndex = (int)s_strMoveIndexX.find(strmove[strMoveDirectionIndex - 1]) + (int)s_strMoveIndexY.find(strmove[strMoveDirectionIndex]) * 8;
		if (strmove.size() - (strMoveDirectionIndex) > 1 && strmove[strMoveDirectionIndex + 1] == '=' 
			&& s_strMoveTypes.find(strmove[strMoveDirectionIndex + 2]) != std::string::npos)
		{
			typeToPromote = (Piece)s_strMoveTypes.find(strmove[strMoveDirectionIndex + 2]);
		}

		move.pieceToMove = typeToMove;
		move.pieceOnDirection = m_Board.GetPieceType(directionIndex);
		move.piecePromote = typeToPromote;

		//check for the posible pieces 
		std::vector<Board::Move> possibleIndexMoves;
		std::vector<Board::Move> possibleMoves;
		m_Board.GetAvailableMoves(possibleMoves, typeToMove);

		for (auto& m : possibleMoves)
		{
			if (m.index + m.move == directionIndex)
				possibleIndexMoves.emplace_back(m);
		}

		if (possibleIndexMoves.size() != 1)
		{
			int strHelperIndex = (typeToMove == PAWN ? 0 : 1) + startIndex;
			int helperIndexX = -1;
			int helperIndexY = -1;

			if (strmove[strHelperIndex] >= 'a' && strmove[strHelperIndex] <= 'h')
			{
				helperIndexX = (int)s_strMoveIndexX.find(strmove[strHelperIndex]);
				if (strMoveDirectionIndex - strHelperIndex > 1 && strmove[strHelperIndex + 1] != 'x')
					helperIndexY = (int)s_strMoveIndexY.find(strmove[strHelperIndex + 1]);
			}
			else
			{
				helperIndexY = (int)s_strMoveIndexY.find(strmove[strHelperIndex]);
				if (strMoveDirectionIndex - strHelperIndex > 1 && strmove[strHelperIndex + 1] != 'x')
					helperIndexX = (int)s_strMoveIndexX.find(strmove[strHelperIndex + 1]);
			}

			bool xok = false, yok = false;

			for (auto& m : possibleIndexMoves)
			{
				xok = false;
				yok = false;

				if (helperIndexX != -1)
				{
					if (m.index % 8 == helperIndexX)
						xok = true;
					else
						continue;
				}
				else
					xok = true;
				
				if (helperIndexY != -1)
				{
					if (m.index / 8 == helperIndexY)
						yok = true;
					else
						continue;
				}
				else
					yok = true;

				if (xok && yok)
				{
					move.move = m;
					return;
				}
			}
		}

		if (possibleIndexMoves.empty())
		{
			//something bad happened
			move.pieceToMove = NONE;
			return;
		}

		move.move = possibleIndexMoves[0];
	}

	void GameManager::ConvertMoveToMoveData(const Board::Move& move, MoveData& moveData, Piece promotedType) const
	{
		moveData.pieceToMove = m_Board.GetPieceType(move.index);
		moveData.pieceOnDirection = m_Board.GetPieceType(move.index + move.move);

		moveData.FiftyMoveCounter = m_Board.GetFiftyMoveCount();
		moveData.lastMoveIndex = m_Board.GetLastMoveIndex();
		moveData.piecePromote = promotedType;

		moveData.K = m_Board.GetKRoke();
		moveData.Q = m_Board.GetQRoke();
		moveData.k = m_Board.GetkRoke();
		moveData.q = m_Board.GetqRoke();

		moveData.move = move;

		if (moveData.pieceToMove == PAWN && moveData.move.index + moveData.move.move == moveData.lastMoveIndex)
			moveData.pieceOnDirection = PAWN;
	}

	void GameManager::GetCurrentPgnMovePath(Pgn_Game::ChessMovesPath*& path) const
	{
		path = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < m_lastMoveKey.size(); i += 2)
		{
			path = &path->children[m_lastMoveKey[i]];
		}
	}

	void GameManager::AddMove(const std::string& strmove, const MoveData& move)
	{
		Pgn_Game::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();
		
		for (int i = 1; i < m_lastMoveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[m_lastMoveKey[i]];
		}

		bool nextExists = false;

		int nextIndex = -1;

		for (int i = m_lastMoveKey[m_lastMoveKey.size() - 1] + 1; i < currentPath->move.size(); i++)
		{
			if (currentPath->move[i] != "child")
			{
				nextExists = true;
				nextIndex = i;
				break;
			}
		}

		if (!nextExists)
		{
			currentPath->move.emplace_back(strmove);
			m_lastMoveKey[m_lastMoveKey.size() - 1] = currentPath->move.size() - 1;

			m_mapMoves[m_lastMoveKey] = move;

			return;
		}

		if (currentPath->move[nextIndex].find(strmove) == 0)
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex;

			m_mapMoves[m_lastMoveKey] = move;

			return;
		}

		int childIndex = 0;

		for (int i = 0; i < nextIndex; i++)
		{
			if (currentPath->move[i] == "child")
				childIndex++;
		}

		int childAmount = 0;

		for (int i = nextIndex + 1; i < currentPath->move.size(); i++)
		{
			if (currentPath->move[i] != "child")
				break;
			else
				childAmount++;
		}

		for (int i = 0; i < childAmount; i++)
		{
			if (currentPath->children[childIndex + i].move[0].find(strmove) == 0)
			{
				m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex + 1 + i;
				m_lastMoveKey.emplace_back(childIndex + i);
				m_lastMoveKey.emplace_back(0);

				m_mapMoves[m_lastMoveKey] = move;

				return;
			}
		}

		currentPath->move.insert(currentPath->move.begin() + nextIndex + 1, "child");
		currentPath->children.insert(currentPath->children.begin() + childIndex + childAmount, Pgn_Game::ChessMovesPath());
		currentPath->children[childIndex + childAmount].move.emplace_back(strmove);
		currentPath->children[childIndex + childAmount].parent = currentPath;

		m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex + childAmount + 1;
		m_lastMoveKey.emplace_back(childIndex + childAmount);
		m_lastMoveKey.emplace_back(0);

		m_mapMoves[m_lastMoveKey] = move;
	}
}