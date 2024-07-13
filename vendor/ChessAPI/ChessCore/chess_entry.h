#pragma once

#include "chess_board.h"
#include "chess_pecie.h"

#include "Opening_Book/OpeningBook.h"

#include "macros.h"
#include "vec2/vec2_lib.h"

#include "pgn/Pgn.h"

#include <functional>

namespace chess
{

	struct PropOfMoves
	{
		chess_core::piece* en_piece;
		uint8_t en_type;
		bool	en_moved;

		chess_core::piece* pa_piece;
		uint8_t pa_type;
		bool	pa_moved;

		vec2<float> pos;
		vec2<float> dir;

		uint8_t fiftyMoveRule;
		
		uint8_t n_pieceType = 0;


		//vec2<float> pos = vec2<float>(0, 0);
		//vec2<float> dir = vec2<float>(0, 0);
		//chess_core::piece* pptr = nullptr;
		//chess_core::piece* eptr = nullptr;
		//chess_core::piece_type_identifier ecn = chess_core::piece_type_identifier::UNKOWN;
		//chess_core::piece_type_identifier pcn = chess_core::piece_type_identifier::UNKOWN;
		//bool p_move = false;
		//bool e_move = false;
		//int new_piece = 0;
		//unsigned int fiftymoverule = 0;
	
		bool operator==(const PropOfMoves& other)
		{
			//if (pos == other.pos && dir == other.dir && pptr == other.pptr && eptr == other.eptr
			//	&& ecn == other.ecn && pcn == other.pcn && p_move == other.p_move && e_move == other.e_move
			//	&& new_piece == other.new_piece)
			//{
			//	return true;
			//}
			//return false;

			if (pos == other.pos && dir == other.dir
				&& en_piece == other.en_piece && en_type == other.en_type && en_moved == other.en_moved
				&& pa_piece == other.pa_piece && pa_type == other.pa_type && pa_moved == other.pa_moved 
				&& fiftyMoveRule == other.fiftyMoveRule
				&& n_pieceType == other.n_pieceType)
			{
				return true;
			}
			return false;
		}
	};
	
	struct PropOfMovesPath
	{
		PropOfMovesPath(PropOfMovesPath* f) : father(f) {}
		PropOfMovesPath() : father(nullptr) {}
		int LastValidMove()
		{
			for(int i = (int)moves.size() - 1; i >= 0; i--)
				if (moves[i].en_type != chess_core::UNKOWN) { return i; }
			return -1;
		}
		void ReloadChildren()
		{
			for (int i = 0; i < children.size(); i++)
			{
				children[i].father = this;
				children[i].ReloadChildren();
			}
		}
		std::vector<PropOfMovesPath> children;
		std::vector<PropOfMoves> moves;
		PropOfMovesPath* father;
	};

	enum VariationMovement
	{
		SWAP = 0,//swap the pointed variation with its parent -- ONLY UP
		END,//swap the pointed variation untill it find a null-parent -- ONLY UP
		INSIDE,//change the order of a group of variations one up or down 
		INSIDE_MAX, //change the order of a group untill the last or the first element
	};

	class chess_entry
	{
	public:
		chess_entry();
		chess_entry(Pgn_Game& pgngame);
		~chess_entry() = default;

		//setting some important values and prepare the programe to run
		void run();

		void rerun(Pgn_Game& pgngame);

		//add new move
		bool make_new_move(vec2<float> piecpos, vec2<float> piecmove);

		//Make sure that move is valid otherwise weird this could happen
		void DeleteMove(std::vector<int> path);

		void PromoteMove(std::vector<int> path, VariationMovement movementType, bool up);

		int get_blockID(vec2<float> pos);

		std::vector<int> get_boardID();

		Pgn_Game& GetPgnGame();

		//path: ECO pgn file path
		std::string GetECO(const std::string& path) const;

		std::string& GetNote(std::vector<int> path);

		Pgn_Game::ChessMovesPath GetMovesByStr() const;

		PropOfMovesPath GetMovesByData() const;

		std::vector<int> GetMovePath() const;

		std::string GetFEN() const;

		std::vector<uint8_t> GetFormatedCurrentPosition() const;

		bool GetPlayerColor();

		std::string GetMoveByUCI(const std::string& uciMove);

		std::vector<vec2<float>> GetPossibleMoves(const vec2<float>& pos);

		bool IsFileLoaded() const;

		bool IsPawnWaiting() const;

		void SetNewPawnType(chess_core::piece_type_identifier pti);

		//playing the next reg move
		void Go_move_Next();

		//going a move back
		void Go_move_Back();

		void Go_Start_Position();

		//You have to be at the start position to use this function
		void Go_next_move_by_path(std::vector<int>& path);

		void Go_move_by_str(const std::string& movename);

		int number_moves = 0;
		bool newVariation = false;

	private:

		bool MoveObject(vec2<float>& pos, vec2<float>& end_move);
		
		//create a new move on the movepath and overwriting the m_pmppath
		void new_move(PropOfMoves& nm, const bool& showverpos, const bool& showhorpos);
		
		//find the next registered move in the same scope and overwriting the m_pmppath 
		void findnextmove(PropOfMovesPath*& p);

		//find the priv registered move and overwriting the m_pmppath
		void findprevmove(PropOfMovesPath*& p);
		
		//converting the src from the (pgn, chess_core) file to a movepath
		void stringpath_to_srcpath(Pgn_Game::ChessMovesPath& cmp, PropOfMovesPath* pmp);

		//getting the current movepath using m_pmppath
		PropOfMovesPath* GetCurMovePath();

	private:
		chess_core::board m_game;
		
		PropOfMoves m_waiting_for_pawn_move;
		std::vector<int> m_pmppath;
		PropOfMovesPath m_movespath;
		Pgn_Game* m_settings;


		std::unordered_map<uint8_t, std::function<void(vec2<float>, chess_core::piece*)>> m_energeticMoveBack;
		std::unordered_map<uint8_t, std::function<void(vec2<float>, chess_core::piece*)>> m_passiveMoveBack;
	};
	
}
