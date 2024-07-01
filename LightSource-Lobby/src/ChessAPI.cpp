#include "ChessAPI.h"

#include "ChessCore/chess_entry.h"

#include "fstream"

static chess::Pgn_File s_PngFile;
static std::string s_PngPath;
static std::string s_PngName;

static std::unordered_map<int, chess::chess_entry> s_Games;
static std::unordered_map<int, std::vector<int>> s_MovePathIntFormat;

static int s_ActiveGame = 0;
static std::vector<int> s_OpenGames;


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

		s_PngFile.CreateGame();

		s_PngPath = "";
		s_PngName = "New Game";

		s_ActiveGame = 0;
		s_OpenGames.push_back(0);
		s_Games[0] = chess::chess_entry(s_PngFile.operator[](0));

		s_Games[0].run();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}

	int GetActiveGame()
	{
		return s_ActiveGame;
	}

	std::vector<int> GetOpenGames()
	{
		return s_OpenGames;
	}

	void SetActiveGame(int index)
	{
		s_ActiveGame = index;
	}

	void CloseOpenGame(int index)
	{
		s_OpenGames.erase(s_OpenGames.begin() + index);
	}

	chess::Pgn_File* GetPgnFile()
	{
		return &s_PngFile;
	}
	chess::Pgn_Game* GetPgnGame()
	{
		return &s_Games[s_ActiveGame].GetPgnGame();
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
		moves = s_Games[s_ActiveGame].GetMovesByStr();
	}
	std::string& GetNote(const std::vector<int>& pathmove)
	{
		return s_Games[s_ActiveGame].GetNote(pathmove);
	}
	std::vector<int>& GetMoveIntFormat()
	{
		return s_MovePathIntFormat[s_ActiveGame];
	}

	std::string GetFEN()
	{
		return s_Games[s_ActiveGame].GetFEN();
	}
	int GetBlockID(int BlockIndex)
	{
		return  s_Games[s_ActiveGame].get_blockID(vec2<float>(BlockIndex - 8 * (int)(BlockIndex / 8), (int)(BlockIndex / 8)));
	}
	std::vector<std::string> GetCurPgnLabelNames()
	{
		return s_Games[s_ActiveGame].GetPgnGame().GetLabelNames();
	}
	std::string& GetCurPgnLabelValue(const std::string& name)
	{
		return s_Games[s_ActiveGame].GetPgnGame()[name];
	}
	bool GetPlayerColor()
	{
		return s_Games[s_ActiveGame].GetPlayerColor();
	}
	std::vector<vec2<float>> GetPossibleDirections(const vec2<float>& pos)
	{
		return s_Games[s_ActiveGame].GetPossibleMoves(pos);
	}
	chess::OpeningBook::PositionID	 GetFormatedPosition()
	{
		return s_Games[s_ActiveGame].GetFormatedCurrentPosition();
	}

	void SetNotePanelPointed(const bool& ispointed)
	{
		m_IsLastPointedPanelNotes = ispointed;
	}

	void  SetNewChessGame(const std::string& path)
	{
		s_Games.clear();
		s_MovePathIntFormat.clear();

		if (path == "")
		{
			//empty game
			s_PngFile = chess::Pgn_File();
			s_PngFile.CreateGame();

			s_PngPath = "";
			s_PngName = "New Game";

			s_Games[0] = chess::chess_entry(s_PngFile.operator[](0));

			s_Games[0].run();
			s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
			return;
		}

		s_PngFile = chess::Pgn_File();

		s_PngPath = path;
		std::ifstream infile(path, std::ios::binary);
		infile >> s_PngFile;
		infile.close();

		size_t index = path.find_last_of("\\") + 1;
		s_PngName = path.substr(index);

		if (!s_PngFile.GetSize())
			s_PngFile.CreateGame();

		s_Games[0] = chess::chess_entry(s_PngFile.operator[](0));

		s_Games[0].run();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
	}

	void SetNewChessGame(int index)
	{
		if (index >= s_PngFile.GetSize() || index < 0)
			return;

		s_ActiveGame = index;
		bool alreadyOpen = false;
		for (int i = 0; i < s_OpenGames.size(); i++)
		{
			if (s_OpenGames[i] == index)
				alreadyOpen = true;
		}
		if(!alreadyOpen)
			s_OpenGames.emplace_back(index);
		s_Games[s_ActiveGame] = chess::chess_entry(s_PngFile.operator[](s_ActiveGame));

		s_Games[s_ActiveGame].run();
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
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
			outfile << s_PngFile;
			outfile.close();

			//auto& pgngame = s_Games[0].GetPgnGame();
			//delete s_Games[0];
			//s_Games[0] = new chess::chess_entry(pgngame);
			//s_Games[0].run();
			//s_MovePathIntFormat[0] = s_Games[0].GetMovePath();

			return;
		}
		std::ofstream outfile(filepath);
		outfile << s_PngFile;
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
		s_PngFile.CreateGame();
		s_ActiveGame = s_PngFile.GetSize() - 1;
		s_OpenGames.emplace_back(s_ActiveGame);
		s_Games[s_ActiveGame] = chess::chess_entry(s_PngFile.operator[](s_ActiveGame));

		s_Games[s_ActiveGame].run();
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
		
	}

	void DeleteGameInFile(int index)
	{
		s_PngFile.DeleteGame(index);

		s_Games.erase(index);
		s_MovePathIntFormat.erase(index);

		if (!s_PngFile.GetSize())
			s_PngFile.CreateGame();
		s_ActiveGame = 0;
		s_Games[0] = chess::chess_entry(s_PngFile.operator[](0));

		s_Games[0].run();
		s_MovePathIntFormat[0] = s_Games[0].GetMovePath();
		
	}

	void SetNewPieceType(int type)
	{
		s_Games[s_ActiveGame].SetNewPawnType(chess_core::piece_type_identifier(type));
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}
	void GoMoveByIntFormat(std::vector<int>& pathmoves)
	{
		s_Games[s_ActiveGame].Go_Start_Position();
		s_Games[s_ActiveGame].Go_next_move_by_path(pathmoves);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}
	void GoMoveByStr(const std::string& movename)
	{
		s_Games[s_ActiveGame].Go_move_by_str(movename);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}
	void NextSavedMove()
	{
		s_Games[s_ActiveGame].Go_move_Next();
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}
	void PreviousSavedMove()
	{
		s_Games[s_ActiveGame].Go_move_Back();
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}


	bool IsMoveValid(glm::vec2 pos, glm::vec2 dir)
	{
		vec2 cpos = vec2(pos.x, pos.y);
		vec2 cdir = vec2(dir.x, dir.y);

		bool retval = s_Games[s_ActiveGame].make_new_move(cpos, cdir - cpos);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
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
		if (s_Games[s_ActiveGame].newVariation)
		{
			s_Games[s_ActiveGame].newVariation = false;
			return true;
		}
		return false;
	}

	void DeleteMove(std::vector<int>& movepath)
	{
		s_Games[s_ActiveGame].DeleteMove(movepath);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}
	void DeleteVariation(std::vector<int>& movepath)
	{
		movepath[movepath.size() - 1] = 0;
		s_Games[s_ActiveGame].DeleteMove(movepath);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}
	void  PromoteVariation(std::vector<int>& movepath)
	{
		s_Games[s_ActiveGame].PromoteMove(movepath, chess::SWAP, true);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetMovePath();
	}

	bool IsWaitingForNewType()
	{
		return s_Games[s_ActiveGame].IsPawnWaiting();
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