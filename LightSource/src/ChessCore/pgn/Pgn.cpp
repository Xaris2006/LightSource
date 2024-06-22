#include "Pgn.h"

#include <cmath>
#include <array>
#include <fstream>

namespace chess
{
	bool IsFileValidFormat(std::string filepath, std::string format)
	{
		size_t pointformatstart = filepath.find_last_of('.');
		std::string fs = filepath.substr(pointformatstart, filepath.size() - pointformatstart + 1);
		if (fs == format)
			return true;
		return false;
	}

	const Pgn_Game::ChessMovesPath Pgn_Game::GetMovePathbyCopy() const
	{
		return m_chessmoves;
	}
	Pgn_Game::ChessMovesPath& Pgn_Game::GetMovePathbyRef()
	{
		return m_chessmoves;
	}
	std::string& Pgn_Game::GetResault()
	{
		return m_resualt;
	}
	std::vector<std::string> Pgn_Game::GetLabelNames() const
	{
		std::vector<std::string> output;
		output.reserve(m_labels.size());
		for (auto& [key, value] : m_labels)
			output.emplace_back(key);
		return output;
	}

	void Pgn_Game::clear()
	{
		m_labels.clear();
		m_chessmoves.children.clear();
		m_chessmoves.details.clear();
		m_chessmoves.move.clear();
		m_resualt = "*";
	}

	std::string& Pgn_Game::operator[](const std::string& label)
	{
		bool finded = false;
		std::string* output = nullptr;
		for (auto& [names, value] : m_labels)
		{
			if (names == label)
			{
				finded = true;
				output = &m_labels[label];
				break;
			}
		}
		if (!finded)
		{
			m_labels[label] = "?";
			output = &m_labels[label];
		}
		return *output;
	}

	std::istream& operator>>(std::istream& stream, Pgn_Game& f)
	{
		std::string data = "";
		while (stream.good())
		{
			std::string input;
			stream >> input;
			data += (input + ' ');
		}
		f.Parse(data);

		return stream;
	}
	std::ostream& operator<<(std::ostream& stream, const Pgn_Game& f)
	{
		std::string output = "";
		bool resultExist = false;
		for (auto& [name, value] : f.m_labels)
		{
			output += "[" + name + " \"" + value + "\"]\n";
			if (name == "Result")
				resultExist = true;
		}
		output += '\n';
		f.WriteMoves(output, f.m_chessmoves);
		if (resultExist)
			output += f.m_labels.at("Result");
		else
			output += '*';
		
		stream << output;
		return stream;
	}

	void Pgn_Game::Parse(std::istream& f)
	{
		std::string data = "";
		while (f.good())
		{
			std::string input;
			f >> input;
			data += (input + ' ');
		}
		Parse(data);
	}

	void  Pgn_Game::Parse(std::string& data)
	{
		if (data.empty())
			return;

		this->data = data;

		bool labelstart = false;
		bool labelvalue = false;
		
		bool labelArea = true;

		bool dollaropen = false;
		bool detailsopen = false;


		std::string labelnamestr = "";
		std::string labelvaluestr = "";

		ChessMovesPath* Parent = &m_chessmoves;
		Parent->move.push_back("");

		for (size_t i = 0; i < data.size(); i++)
		{
			if (labelArea)
			{
				if (data[i] == '\n')
					continue;

				if (!labelvalue)
				{
					if (!labelstart && data[i] != '[')
					{
						i -= 1;
						labelArea = false;
						continue;
					}

					if (data[i] == '[' && !labelstart)
					{
						labelstart = true;
						labelnamestr.clear();
						labelvaluestr.clear();
						continue;
					}

					if (data[i] == ']' && labelstart)
					{
						labelstart = false;
						m_labels[labelnamestr] = labelvaluestr;
						continue;
					}

					if (data[i] == '"')
					{
						labelvalue = true;
						continue;
					}
				}

				if (data[i] == '"' && labelvalue)
				{
					labelvalue = false;
					continue;
				}
				
				if (labelvalue)
				{
					labelvaluestr += data[i];
					continue;
				}

				if (labelstart)//!labelvalue
				{
					if (data.size() > i + 1 && data[i + 1] == '"')
						continue;
					labelnamestr += data[i];
					continue;
				}
			}

			//--moveArea--

			//dollar???
			{
				if (data[i] == '$')
				{
					dollaropen = true;
					continue;
				}

				if (dollaropen)
				{
					if (data[i] < '0' || data[i] > '9')
					{
						dollaropen = false;
						i--;
					}
					continue;
				}
			}

			//notes
			{
				if (data[i] == '{' && !detailsopen)
				{
					detailsopen = true;
					continue;
				}

				if (data[i] == '}')
				{
					detailsopen = false;
					continue;
				}

				if (detailsopen)
				{
					if (Parent->move.empty())
						continue;
					Parent->details[Parent->move.size() -1] += data[i];
					continue;
				}
			}

			//variant
			{
				if (data[i] == '(')
				{
					if (Parent->move[Parent->move.size() - 1].empty())
						Parent->move.resize(Parent->move.size() - 1);

					Parent->children.push_back(ChessMovesPath(Parent));
					Parent->move.push_back("child");
					Parent = &Parent->children[Parent->children.size() - 1];

					Parent->move.push_back("");
					continue;
				}

				if (data[i] == ')')
				{
					if (Parent->move[Parent->move.size() - 1].empty())
						Parent->move.resize(Parent->move.size() -1);

					int index1 = Parent->move[Parent->move.size() - 1].rfind('1');
					int indexDash = Parent->move[Parent->move.size() - 1].find('-');

					if (index1 != std::string::npos && indexDash != std::string::npos && std::abs(index1 - indexDash) == 1)
					{
						//detail??
						Parent->move.resize(Parent->move.size() - 1);
					}

					Parent = Parent->parent;
					Parent->move.push_back("");

					continue;
				}
			}

			//moves
			{
				if (data[i] == '\n' && data[i - 1] == ' ' && data[i-2] == '.')
					continue;

				if ((data[i] == ' ' || data[i] == '\n') && data[i - 1] != '.')
				{
					if (!Parent->move.empty() && Parent->move[Parent->move.size() - 1].empty())
					{
						continue;
					}

					Parent->move.push_back("");
					continue;
				}

				if (data[i] == '.' && data[i - 1] == '.')
				{
					Parent->move[Parent->move.size() - 1] = "";
					i+=2;
					continue;
				}

				if (data[i] == '\n')
				{
					Parent->move[Parent->move.size() - 1] += ' ';
					continue;
				}

				Parent->move[Parent->move.size() - 1] += data[i];
			}

		}

		if (Parent->move[Parent->move.size() - 1].empty())
			Parent->move.resize(Parent->move.size() - 1);

		if (Parent->move[Parent->move.size() - 1].find('-') != std::string::npos && Parent->move[Parent->move.size() - 1].find('1') != std::string::npos)
		{
			m_resualt = Parent->move[Parent->move.size() - 1];
			Parent->move.resize(Parent->move.size() - 1);
		}

		func_delete_BC_stuff(&m_chessmoves);

		if (m_labels["Result"] == "?")
			m_resualt = '*';
		else
			m_resualt = m_labels["Result"];
	}

#if 0
	void Pgn_Game::Parse(std::string& data)
	{
		if (!data.size()) { return; }

		if (data[data.size() - 1] == ' ') { data.resize(data.size() - 1); }

		if (!data.size()) { return; }
		this->data = data;

		size_t index = 0;
		bool labelstart = false;
		bool labelvalue = false;
		bool dollaropen = false;
		bool detailsopen = false;

		std::string labelnamestr = "";
		std::string* labelvalueptr;


		ChessMovesPath* Parent = &m_chessmoves;
		m_chessmoves.move.push_back("");

		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i] == '{' && !labelstart && !labelvalue)
			{
				dollaropen = false;
				detailsopen = true;

				Parent->details[Parent->move.size() - 1] = { "" };

				continue;
			}
			else if (data[i] == '}' && !labelstart && !labelvalue)
			{
				dollaropen = false;
				detailsopen = false;

				continue;
			}
			else if (data[i] == '(' && !detailsopen && !labelstart && !labelvalue)
			{
				dollaropen = false;

				Parent->children.push_back(ChessMovesPath(Parent));
				Parent->move.push_back("child");
				Parent = &Parent->children[Parent->children.size() - 1];
				if (data[i + 1] != ' ' && data[i + 1] != '\n' && data[i + 1] != '{' && data[i + 1] != '$') { Parent->move.push_back(""); }

				continue;
			}
			else if (data[i] == ')' && !detailsopen && !labelstart && !labelvalue)
			{
				Parent = Parent->parent;

				dollaropen = false;

				continue;
			}
			else if (data[i] == '[' && !detailsopen && !labelvalue)
			{
				dollaropen = false;
				labelstart = true;

				labelnamestr = "";

				continue;
			}
			else if (data[i] == ']' && !detailsopen && !labelvalue)
			{
				dollaropen = false;
				labelstart = false;

				continue;
			}
			else if (data[i] == '$' && !detailsopen && !labelstart && !labelvalue)
			{
				dollaropen = true;

				continue;
			}
			else if ((data[i] == ' ' /* || data[i] == '\n'*/) && !labelvalue && !detailsopen)
			{
				if (dollaropen)
				{
					dollaropen = false;
				}
				if (labelstart)
				{
					if (data[i + 1] == '"')
						continue;
				}
				else if (data[i - 1] != '.' && data[i + 1] != '[' && data[i + 1] != '{' && data[i + 1] != '(' && data[i + 1] != ')' && data[i + 1] != '$' && data[i-1] != ' ')
				{
					if (Parent->move.size())
					{ 
						if (Parent->move[Parent->move.size() - 1] == "")
						{
							continue; 
						} 
					}
					Parent->move.push_back("");
					continue;
				}
				else if (data[i - 1] == '.')
				{

				}
				else
					continue;
			}

			if (detailsopen)
			{
				if (Parent->move.size()) { Parent->details[Parent->move.size() - 1] += data[i]; }
			}
			else if (labelvalue)
			{
				if (data[i] == '"') { labelvalue = false; continue; }
				*labelvalueptr += data[i];
			}
			else if (labelstart)
			{
				if (data[i] == '"') { labelvalue = true; labelvalueptr = &m_labels[labelnamestr]; *labelvalueptr = ""; continue; }
				labelnamestr += data[i];
			}
			else if (!dollaropen)
			{
				if (Parent->move.size())
				{
					if (data[i] == '\n')//htan '\n' anti ' '
						Parent->move[Parent->move.size() - 1] += ' ';
					else
						Parent->move[Parent->move.size() - 1] += data[i];
				}
			}
		}
		if (!m_chessmoves.move.size()) { return; }
		if (m_chessmoves.move[m_chessmoves.move.size() - 1] == "") { m_chessmoves.move.resize(m_chessmoves.move.size() - 1); }
		if (!m_chessmoves.move.size()) { return; }
		if (m_chessmoves.move[m_chessmoves.move.size() - 1][m_chessmoves.move[m_chessmoves.move.size() - 1].size() / 2] == '-' && m_chessmoves.move[m_chessmoves.move.size() - 1] != "0-0")
		{
			m_resualt = m_chessmoves.move[m_chessmoves.move.size() - 1];
			m_chessmoves.move.resize(m_chessmoves.move.size() - 1);
		}

		else if (m_chessmoves.move[m_chessmoves.move.size() - 1] == "*") { m_chessmoves.move.resize(m_chessmoves.move.size() - 1); }

		func_delete_BC_stuff(&m_chessmoves);

		if (m_labels["Result"] == "?")
			m_resualt = '*';
		else
			m_resualt = m_labels["Result"];
	}

#endif

	void Pgn_Game::WriteMoves(std::string& op, ChessMovesPath par) const
	{
		int index = 0;
		for (int i = 0; i < par.move.size(); i++)
		{
			if (par.move[i] == "child") { op += "( "; WriteMoves(op, par.children[index]); op += ") "; index += 1; continue; }
			else
			{
				op += par.move[i] + " ";

				if (par.details[i] != "")
					op += ("{ " + par.details[i] + " } ");
			}
		}
	}

	bool Pgn_Game::func_delete_BC_stuff(ChessMovesPath* cur_Parent)
	{
		const std::array<std::string, 1> CB_weird_moves = { "Z0" };
		int index_of_children = 0;
		for (int i = 0; i < cur_Parent->move.size(); i++)
		{
			if (cur_Parent->move[i].find("...") != std::string::npos)
			{
				cur_Parent->move[i] = cur_Parent->move[i].substr(cur_Parent->move[i].find("...") + 4);
			}
			else if (cur_Parent->move[i] == "child")
			{
				if (!func_delete_BC_stuff(&cur_Parent->children[index_of_children]))
				{
					cur_Parent->children.erase(cur_Parent->children.begin() + index_of_children);
					cur_Parent->move.erase(cur_Parent->move.begin() + i);
					i -= 1;
				}
				else
					index_of_children += 1;
			}

			for (int j = 0; j < CB_weird_moves.size(); j++)
			{
				if (cur_Parent->move[i].find(CB_weird_moves[j]) != std::string::npos)
				{
					cur_Parent->move.resize(i);
					cur_Parent->children.resize(index_of_children);
					if (i == 0)
						return false;
					return true;
				}
			}
		}
		return true;
	}


	//Pgn_File

	std::istream& operator>>(std::istream& stream, Pgn_File& f)
	{
		stream.seekg(0, std::ios_base::end);
		std::streampos maxSize = stream.tellg();
		stream.seekg(0, std::ios_base::beg);
		
		if (maxSize <= 0) { return stream; }
		char* data = new char[maxSize];
		stream.read(data, maxSize);

		std::string all_data;
		for (int i = 0; i < maxSize; i++)
		{
			if (data[i] == '\r')
				continue;
			all_data += data[i];
		}
		delete[] data;

		int start_index = -1;
		for (int i = 0; i < all_data.size(); i++)
		{
			if (all_data[i] >= 0) { start_index = i; break; }
		}
		if (all_data.size() - start_index <= 2) { return stream; }

		f.m_Games.reserve(maxSize / 800);

		std::string current_data = "";
		current_data += all_data[start_index];
		current_data += all_data[start_index + 1];

		bool textopen = false;
		bool noteopen = false;

		for (int i = start_index + 2; i < all_data.size(); i++)
		{
			if (all_data[i] == '{') { noteopen = true; }
			if (all_data[i] == '}') { noteopen = false; }

			if (all_data[i] == '[' && all_data[i - 1] == '\n' && all_data[i - 2] != ']' && !noteopen)
			{
				f.m_Games.emplace_back(Pgn_Game());
				f.m_Games.operator[](f.m_Games.size() - 1).Parse(current_data);
				current_data = '[';
				continue;
			}
			current_data += all_data[i];
		}
		char* startptr = &current_data[0];
		f.m_Games.emplace_back(Pgn_Game());
		f.m_Games.operator[](f.m_Games.size() - 1).Parse(current_data);
		
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const Pgn_File& f)
	{
		for (auto& game : f.m_Games)
		{
			stream << game << '\n';
		}
		return stream;
	}

	void Pgn_File::SearchLabel(std::vector<int>& possitiveIndexes, int& searched, const std::map<std::string, std::string>& settings)
	{
		if (settings.empty())
		{
			searched = m_Games.size();
			return;
		}

		bool doubleBreak = false;
		int correct = 0;
		searched = 0;
		for (int i = 0; i < GetSize(); i++)
		{
			for (auto& existedLabelName : m_Games[i].GetLabelNames())
			{
				for (auto& [searchLabelName, LabelValue] : settings)
				{
					if (existedLabelName == searchLabelName)
					{
						if (m_Games[i][searchLabelName].find(LabelValue) + 1)
							correct += 1;
						else
							doubleBreak = true;
						break;
					}
				}
				if (doubleBreak)
				{
					doubleBreak = false;
					break;
				}
				if (correct == settings.size())
				{
					possitiveIndexes.push_back(i);
					break;
				}
			}
			correct = 0;
			searched += 1;
		}
	}


	void Pgn_File::SearchMoves(std::vector<int>& possitiveIndexes, int& searched, const std::vector<std::string>& moves)
	{
		bool wrong = true;
		searched = 0;
		if (moves.empty())
			return;
		for (int i = 0; i < GetSize(); i++)
		{
			for (int j = 0; j < m_Games[i].GetMovePathbyRef().move.size() && j < moves.size(); j++)
			{
				wrong = false;
				if (m_Games[i].GetMovePathbyRef().move[j] != moves[j])
				{
					wrong = true;
					break;
				}
			}
			if (!wrong)
				possitiveIndexes.push_back(i);
			searched += 1;
			wrong = true;
		}
	}

	size_t Pgn_File::GetSize() const
	{
		return m_Games.size();
	}
	Pgn_Game& Pgn_File::operator[] (size_t index)
	{
		return m_Games[index];
	}

	void Pgn_File::CreateGame(int index)
	{
		if (index == -1)
		{
			m_Games.push_back(Pgn_Game());
			return;
		}
		m_Games.insert(m_Games.begin() + index, Pgn_Game());
	}
	void Pgn_File::DeleteGame(size_t index)
	{
		m_Games.erase(m_Games.begin() + index);
	}
	void Pgn_File::MoveGame(size_t position, size_t direction)
	{
		m_Games.insert(m_Games.begin() + direction, m_Games[position]);
		m_Games.erase(m_Games.begin() + direction);
	}
}
