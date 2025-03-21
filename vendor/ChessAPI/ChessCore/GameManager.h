#pragma once

#include "Board.h"
#include "pgn/Pgn.h"

namespace Chess
{
	class GameManager
	{
	public:
		using MoveKey = std::vector<int>;

		struct MoveKey_Hash
		{
			size_t operator() (const MoveKey& pos) const
			{
				std::size_t hash = 0;
				std::hash<int> hasher;
				for (const int& i : pos)
				{
					// Combine the hash values (simple XOR + shift approach)
					hash ^= hasher(i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
				}
				return hash;
			}
		};

		struct PieceID
		{
			Piece type = NONE;
			Color color;
		};

		struct MoveData
		{
			Board::Move move;
			Piece pieceToMove;
			Piece pieceOnDirection = NONE;
			Piece piecePromote = NONE;

			int lastMoveIndex = -1;

			bool K = true, Q = true, k = true, q = true;

			int FiftyMoveCounter = 0;

			bool operator==(const MoveData& other) const
			{
				if (move.index == other.move.index && move.move == other.move.move
					&& pieceToMove == other.pieceToMove && pieceOnDirection == other.pieceOnDirection && piecePromote == other.piecePromote
					&& lastMoveIndex == other.lastMoveIndex && FiftyMoveCounter == other.FiftyMoveCounter
					&& K == other.K && Q == other.Q && k == other.k && q == other.q)
				{
					return true;
				}
			
				return false;
			}
		};

		enum VariationEdit
		{
			SWAP = 0,	 //swap the pointed variation with its parent
			SWAP_MAIN,	 //swap the pointed variation until it find a null-parent
			INSIDE_UP,	 //change the order of a group so the pointed variation go one up 
			INSIDE_DOWN, //change the order of a group so the pointed variation go one down 
			INSIDE_MAX,  //change the order of a group so the pointed variation to be first
			INSIDE_MIN,  //change the order of a group so the pointed variation to be last
		};

	public:
		GameManager() = default;
		~GameManager();

		void InitPgnGame(PgnGame& pgnGame);
		
		GameManager& operator=(const GameManager& other) = delete;
		void Clear();

		void GetFen(std::string& fen) const;
		std::string GetFen() const;
		std::vector<uint8_t> GetFormatedFEN() const;
		
		Color GetPlayerToPlay() const;

		PieceID GetPieceID(int index) const;
		PieceID GetPieceID(int indexX, int indexY) const;

		//path: ECO pgn file path
		//std::string GetECO(const std::string& path) const;
		std::string& GetNote(const MoveKey& moveKey);
		//PropOfMovesPath GetMovesByData() const;

		PgnGame::ChessMovesPath GetMovesByStr() const;
		MoveKey GetLastMoveKey() const;
		
		void GetAvailableMoves(std::vector<Board::Move>& moves) const;

		bool IsMoveValid(Board::Move move) const;

		Board::MakeMoveStatus MakeMove(Board::Move move, Piece promotedType = NONE);
		Board::MakeMoveStatus MakeMove(const std::string& move);

		void GoNextMove();
		void GoPreviusMove();
		void GoInitialPosition();
		void GoToPositionByKey(const MoveKey& moveKey);

		void EditVariation(const MoveKey& moveKey, VariationEdit editType);
		void DeleteMove(const MoveKey& moveKey); //or the whole variation if moveKey.last == 0

		PgnGame& GetPgnGame() { return *m_pgnGame; }

		std::string ConvertUCIStringToString(const std::string& uciMove) const;

	private:
		void ConvertMoveDataToString(const MoveData& move, std::string& strmove) const;
		void ConvertStringToMoveData(const std::string& strmove, MoveData& move) const;
		void ConvertMoveToMoveData(const Board::Move& move, MoveData& moveData, Piece promotedType = NONE) const;

		void GetCurrentPgnMovePath(PgnGame::ChessMovesPath*& path) const;

		void AddMove(const std::string& strmove, const MoveData& move);

	private:
		Board m_Board;

		PgnGame* m_pgnGame = nullptr;
		std::unordered_map<MoveKey, MoveData, MoveKey_Hash> m_mapMoves;
		MoveKey m_lastMoveKey;
	};
}
