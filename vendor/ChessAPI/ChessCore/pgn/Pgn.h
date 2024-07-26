#pragma once

#ifndef PGN_PARSER
#define PGN_PARSER

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

namespace chess
{
	bool IsFileValidFormat(std::string filepath, std::string format);

	enum FileFormat
	{
		CHESS,
		PGN
	};

	enum RunningType
	{
		SINGLE,
		MULTY
	};

	//a single game pgn viewer
	class Pgn_Game
	{
	public:
		struct ChessMovesPath
		{
			ChessMovesPath() = default;
			ChessMovesPath(ChessMovesPath* p) : parent(p) {};

			void ReloadChildren()
			{
				for (int i = 0; i < children.size(); i++)
				{
					children[i].parent = this;
					children[i].ReloadChildren();
				}
			}

			ChessMovesPath* parent = nullptr;
			std::vector<ChessMovesPath> children;
			std::unordered_map<int, std::string> details;
			std::vector<std::string> move;
		};

	public:
		Pgn_Game() = default;
		~Pgn_Game() = default;

		const ChessMovesPath GetMovePathbyCopy() const;
		ChessMovesPath& GetMovePathbyRef();
		std::string& GetResault();
		std::vector<std::string> GetLabelNames() const;
		std::string& GetFirstNote();

		void clear();
		std::string& operator[](const std::string& label);

		friend std::istream& operator>>(std::istream& stream, Pgn_Game& f);
		friend std::ostream& operator<<(std::ostream& stream, const Pgn_Game& f);

		void Parse(std::istream& f);

		void Parse(std::string& data);

	private:

		void WriteMoves(std::string& op, ChessMovesPath par) const;

		static bool func_delete_BC_stuff(ChessMovesPath* cur_Parent);

	private:
		
		std::map<std::string, std::string> m_labels;
		ChessMovesPath m_chessmoves;
		std::string m_resualt = "*";
		std::string m_firstNote = "";

		//for debug
		//std::string data;
	};

	class Pgn_File
	{
	public:

		Pgn_File() = default;
		~Pgn_File() = default;

		//stream should be opened by binary mode
		friend std::istream& operator>>(std::istream& stream, Pgn_File& f);

		friend std::ostream& operator<<(std::ostream& stream, const Pgn_File& f);

		void SearchLabel(std::vector<int>& possitiveIndexes, int& searched, const std::map<std::string, std::string>& settings);
		void SearchMoves(std::vector<int>& possitiveIndexes, int& searched, const std::vector<std::string>& moves);

		size_t GetSize() const;
		Pgn_Game& operator[] (size_t index);

		void CreateGame(int index = -1); //index = -1 -> placed at the end
		void DeleteGame(size_t index);
		void MoveGame(size_t position, size_t direction);

	private:
		std::vector<Pgn_Game> m_Games;
	};

}

#endif // PGN
