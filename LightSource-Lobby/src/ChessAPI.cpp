#include "ChessAPI.h"

#include "ChessCore/chess_entry.h"

#include "fstream"

static chess::Pgn_File* s_PngFile = nullptr;
static std::string s_PngPath;
static std::string s_PngName;

static std::unordered_map<int, chess::chess_entry> s_Games;
static std::unordered_map<int, std::vector<int>> s_MovePathIntFormat;

static std::string s_PieceName[7];
static std::array<int, 64> s_OldMovePath;

static bool m_IsLastPointedPanelNotes = false;

namespace ChessAPI
{
	void Init()
	{
		s_PieceName[0] = "";
		s_PieceName[1] = "";
		s_PieceName[2] = "K";
		s_PieceName[3] = "B";
		s_PieceName[4] = "R";
		s_PieceName[5] = "Q";
		s_PieceName[6] = "K";

		s_PngFile = new chess::Pgn_File();
		s_PngFile->CreateGame();

		s_PngPath = "";
		s_PngName = "New Game";

		s_Games[0] = chess::chess_entry(s_PngFile->operator[](0));

		s_Games[0].run();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}


	chess::Pgn_File* GetPgnFile()
	{
		return s_PngFile;
	}
	chess::Pgn_Game* GetPgnGame()
	{
		return &s_Games[0].GetPgnGame();
	}

	std::string GetPieceName(int type)
	{
		return s_PieceName[type];
	}
	std::string& GetPgnFileName()
	{
		return s_PngName;
	}
	std::string& GetPgnFilePath()
	{
		return s_PngPath;
	}
	void GetMovesPgnFormat(chess::Pgn_Game::ChessMovesPath& moves)
	{
		moves = s_Games[0].GetMovesByStr();
		//m_BoardID = s_Games[0].get_boardID();
	}
	std::string& GetNote(const std::vector<int>& pathmove)
	{
		return s_Games[0].GetNote(pathmove);
	}
	std::vector<int>& GetMoveIntFormat()
	{
		return s_MovePathIntFormat[0];
	}

	std::string GetFEN()
	{
		return s_Games[0].GetFEN();
	}
	int GetBlockID(int BlockIndex)
	{
		return  s_Games[0].get_blockID(vec2<float>(BlockIndex - 8 * (int)(BlockIndex / 8), (int)(BlockIndex / 8)));
	}
	std::vector<std::string> GetCurPgnLabelNames()
	{
		return s_Games[0].GetPgnGame().GetLabelNames();
	}
	std::string& GetCurPgnLabelValue(const std::string& name)
	{
		return s_Games[0].GetPgnGame()[name];
	}
	bool GetPlayerColor()
	{
		return s_Games[0].GetPlayerColor();
	}
	std::vector<vec2<float>> GetPossibleDirections(const vec2<float>& pos)
	{
		return s_Games[0].GetPossibleMoves(pos);
	}
	chess::OpeningBook::PositionID	 GetFormatedPosition()
	{
		return s_Games[0].GetFormatedCurrentPosition();
	}

	void SetNotePanelPointed(const bool& ispointed)
	{
		m_IsLastPointedPanelNotes = ispointed;
	}

	void  SetNewChessGame(const std::string& path)
	{
		if (s_PngFile)
		{
			delete s_PngFile;
			s_PngFile = nullptr;
		}
		s_Games.clear();
		s_MovePathIntFormat.clear();

		if (path == "")
		{
			//empty game
			s_PngFile = new chess::Pgn_File();
			s_PngFile->CreateGame();

			s_PngPath = "";
			s_PngName = "New Game";

			s_Games[0] = chess::chess_entry(s_PngFile->operator[](0));

			s_Games[0].run();
			s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
			return;
		}

		s_PngFile = new chess::Pgn_File();

		s_PngPath = path;
		std::ifstream infile(path, std::ios::binary);
		infile >> *s_PngFile;
		infile.close();

		size_t index = path.find_last_of("\\") + 1;
		s_PngName = path.substr(index);

		if (!s_PngFile->GetSize())
			s_PngFile->CreateGame();

		s_Games[0] = chess::chess_entry(s_PngFile->operator[](0));

		s_Games[0].run();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}

	void SetNewChessGame(int index)
	{
		if (index >= s_PngFile->GetSize() || index < 0)
			return;

		s_Games[0] = chess::chess_entry(s_PngFile->operator[](index));

		s_Games[0].run();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}

	void OverWriteChessFile(const std::string& filepath)
	{
		if (filepath == "")
		{
			if (s_PngPath == "")
			{
				auto& pgngame = s_Games[0].GetPgnGame();
				s_Games.clear();
				s_MovePathIntFormat.clear();
				s_Games[0] = chess::chess_entry(pgngame);
				s_Games[0].run();
				s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
				return;
			}
			std::ofstream outfile(s_PngPath);
			if (s_PngFile)
				outfile << *s_PngFile;
			outfile.close();

			//auto& pgngame = s_Games[0].GetPgnGame();
			//delete s_Games[0];
			//s_Games[0] = new chess::chess_entry(pgngame);
			//s_Games[0].run();
			//s_MovePathIntFormat[0] = s_Games[0].GetMovePath();

			return;
		}
		std::ofstream outfile(filepath);
		if (s_PngFile)
			outfile << *s_PngFile;
		s_PngPath = filepath;
		outfile.close();

		//auto& pgngame = s_Games[0].GetPgnGame();
		//delete s_Games[0];
		//s_Games[0] = new chess::chess_entry(pgngame);
		//s_Games[0].run();
		//s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}

	void NewGameInFile()
	{
		if (s_PngFile)
		{
			s_PngFile->CreateGame();
			s_Games[0] = chess::chess_entry(s_PngFile->operator[](s_PngFile->GetSize() - 1));

			s_Games[0].run();
			s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
		}
	}

	void DeleteGameInFile(int index)
	{
		if (s_PngFile)
		{
			s_PngFile->DeleteGame(index);

			s_Games.erase(index);

			if (!s_PngFile->GetSize())
				s_PngFile->CreateGame();
			s_Games[0] = chess::chess_entry(s_PngFile->operator[](0));

			s_Games[0].run();
			s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
		}
	}

	void SetNewPieceType(int type)
	{
		s_Games[0].SetNewPawnType(chess_core::piece_type_identifier(type));
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}
	void GoMoveByIntFormat(std::vector<int>& pathmoves)
	{
		s_Games[0].Go_Start_Position();
		s_Games[0].Go_next_move_by_path(pathmoves);
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}
	void GoMoveByStr(const std::string& movename)
	{
		s_Games[0].Go_move_by_str(movename);
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}
	void NextSavedMove()
	{
		s_Games[0].Go_move_Next();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}
	void PreviousSavedMove()
	{
		s_Games[0].Go_move_Back();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}


	bool IsMoveValid(glm::vec2 pos, glm::vec2 dir)
	{
		vec2 cpos = vec2(pos.x, pos.y);
		vec2 cdir = vec2(dir.x, dir.y);

		bool retval = s_Games[0].make_new_move(cpos, cdir - cpos);
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
		return retval;
	}

	bool IsBoardChanged()
	{
		bool output = false;
		for (int i = 0; i < 64; i++)
		{
			int id = GetBlockID(i);
			if (s_OldMovePath[i] != id)
			{
				s_OldMovePath[i] = id;
				output = true;
			}
		}
		return output;
	}

	bool IsNewVariationAdded()
	{
		if (s_Games[0].newVariation)
		{
			s_Games[0].newVariation = false;
			return true;
		}
		return false;
	}

	void DeleteMove(std::vector<int>& movepath)
	{
		s_Games[0].DeleteMove(movepath);
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}
	void DeleteVariation(std::vector<int>& movepath)
	{
		movepath[movepath.size() - 1] = 0;
		s_Games[0].DeleteMove(movepath);
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}
	void  PromoteVariation(std::vector<int>& movepath)
	{
		s_Games[0].PromoteMove(movepath, chess::SWAP, true);
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}

	bool IsWaitingForNewType()
	{
		return s_Games[0].IsPawnWaiting();
	}

	//void ReverseBoard()
	//{
	//	m_reverse_boardType = !m_reverse_boardType;
	//}
	//bool GetReverseBoardType()
	//{
	//	return m_reverse_boardType;
	//}

}