#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace Chess
{
	bool IsFileValidFormat(std::string filepath, std::string format);

	class PgnGame
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
		PgnGame() = default;
		~PgnGame();

		const ChessMovesPath GetMovePathbyCopy() const;
		ChessMovesPath& GetMovePathbyRef();
		std::string& GetResault();
		std::vector<std::string> GetLabelNames() const;
		std::string& GetFirstNote();

		void RemoveLabel(const std::string& name);

		void AddReference();
		void RemoveReference();

		bool IsFree() const;

		void Clear();
		std::string& operator[](const std::string& label);

		std::string GetData() const;

		std::string GetDataRead() const;

		friend std::istream& operator>>(std::istream& stream, PgnGame& f);

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
		int m_Count = 0;

		std::string m_DataRead;
	};



}
