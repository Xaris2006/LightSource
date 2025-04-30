#include "PgnGame.h"

#include <array>
#include <fstream>

namespace Chess
{
	bool IsFileValidFormat(std::string filepath, std::string format)
	{
		size_t pointformatstart = filepath.find_last_of('.');
		std::string fs = filepath.substr(pointformatstart, filepath.size() - pointformatstart + 1);
		if (fs == format)
			return true;
		return false;
	}

	PgnGame::~PgnGame()
	{
		if (m_Count)
			__debugbreak();
	}

	const PgnGame::ChessMovesPath PgnGame::GetMovePathbyCopy() const
	{
		return m_chessmoves;
	}

	PgnGame::ChessMovesPath& PgnGame::GetMovePathbyRef()
	{
		return m_chessmoves;
	}

	std::string& PgnGame::GetResault()
	{
		return m_resualt;
	}

	std::vector<std::string> PgnGame::GetLabelNames() const
	{
		std::vector<std::string> output;
		output.reserve(m_labels.size());
		for (auto& [key, value] : m_labels)
			output.emplace_back(key);
		return output;
	}

	std::string& PgnGame::GetFirstNote()
	{
		return m_firstNote;
	}

	void PgnGame::AddReference()
	{
		m_Count++;
	}

	void PgnGame::RemoveReference()
	{
		m_Count--;
		if (m_Count < 0)
		{
			__debugbreak();
			m_Count = 0;
		}
	}

	bool PgnGame::IsFree() const
	{
		return m_Count == 0;
	}

	void PgnGame::Clear()
	{
		m_firstNote = "";
		//m_Count = 0;
		m_DataRead = "";

		m_labels.clear();
		m_chessmoves.children.clear();
		m_chessmoves.details.clear();
		m_chessmoves.move.clear();
		m_resualt = "*";
	}

	void PgnGame::RemoveLabel(const std::string& name)
	{
		m_labels[name] = "";
		m_labels.erase(name);
	}

	std::string& PgnGame::operator[](const std::string& label)
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

	std::string PgnGame::GetData() const
	{
		std::string output = "";
		bool resultExist = false;
		for (auto& [name, value] : m_labels)
		{
			output += "[" + name + " \"" + value + "\"]\n";
			if (name == "Result")
				resultExist = true;
		}
		output += '\n';

		if (!m_firstNote.empty())
			output += ('{' + m_firstNote + "} ");

		WriteMoves(output, m_chessmoves);
		if (resultExist && m_labels.at("Result") != "?")
			output += m_labels.at("Result");
		else
			output += '*';

		output += '\n';

		return output;
	}

	std::string PgnGame::GetDataRead() const
	{
		return m_DataRead;
	}

	std::istream& operator>>(std::istream& stream, PgnGame& f)
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

	void PgnGame::Parse(std::istream& f)
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

	void PgnGame::Parse(std::string& data, bool onlyRead, bool readMoves)
	{
		Clear();

		if (data.empty())
			return;

		bool labelstart = false;
		bool labelvalue = false;

		bool labelArea = true;

		bool dollaropen = false;
		bool detailsopen = false;


		std::string labelnamestr = "";
		std::string labelvaluestr = "";

		ChessMovesPath* Parent = &m_chessmoves;

		if (readMoves)
		{
			Parent->details.reserve(20);
			Parent->move.reserve(80);
			Parent->move.emplace_back("");
		}
		
		for (size_t i = 0; i < data.size(); i++)
		{
			if (labelArea)
			{
				if (data[i] == '\n')
					continue;

				if (data[i] == '\\')
				{
					i++;
					continue;
				}

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

					if (data[i] == ']' && labelstart && data[i - 1])
					{
						labelstart = false;
						m_labels[labelnamestr] = labelvaluestr;
						continue;
					}

					if (data[i] == '"' && data[i - 1])
					{
						labelvalue = true;
						continue;
					}
				}

				if (data[i] == '"' && labelvalue && data[i - 1])
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

			if (!readMoves)
				break;

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

					if (Parent->move[0] == "" && !Parent->parent)
					{
						m_firstNote += data[i];
						continue;
					}

					Parent->details[Parent->move.size() - 1] += data[i];
					continue;
				}
			}

			//variant
			{
				if (data[i] == '(')
				{
					if (Parent->move[Parent->move.size() - 1].empty())
						Parent->move.resize(Parent->move.size() - 1);

					Parent->children.emplace_back(ChessMovesPath(Parent));
					Parent->move.emplace_back("child");
					Parent = &Parent->children[Parent->children.size() - 1];

					Parent->details.reserve(20);
					Parent->move.reserve(30);
					Parent->move.emplace_back("");
					continue;
				}

				if (data[i] == ')')
				{
					if (Parent->move[Parent->move.size() - 1].empty())
						Parent->move.resize(Parent->move.size() - 1);

					int index1 = Parent->move[Parent->move.size() - 1].rfind('1');
					int indexDash = Parent->move[Parent->move.size() - 1].find('-');

					if (index1 != std::string::npos && indexDash != std::string::npos && std::abs(index1 - indexDash) == 1)
					{
						//detail??
						Parent->move.resize(Parent->move.size() - 1);
					}

					Parent = Parent->parent;
					Parent->move.emplace_back("");

					continue;
				}
			}

			//moves
			{
				if (data[i] == '\n' && data[i - 1] == ' ' && data[i - 2] == '.')
					continue;

				if ((data[i] == ' ' || data[i] == '\n') && data[i - 1] != '.')
				{
					if (!Parent->move.empty() && Parent->move[Parent->move.size() - 1].empty())
					{
						continue;
					}

					Parent->move.emplace_back("");
					continue;
				}

				if (data[i] == '.' && data[i - 1] == '.')
				{
					Parent->move[Parent->move.size() - 1] = "";
					i += 2;
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

		if (Parent->move.empty())
			return;

		if (Parent->move[Parent->move.size() - 1].empty())
			Parent->move.resize(Parent->move.size() - 1);

		if (Parent->move.empty())
			return;

		if (Parent->move[Parent->move.size() - 1] == "*")
			Parent->move.resize(Parent->move.size() - 1);

		if (Parent->move.empty())
			return;

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

		if(!onlyRead)
			m_DataRead = GetData();
	}

	void PgnGame::WriteMoves(std::string& op, ChessMovesPath par) const
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

	bool PgnGame::func_delete_BC_stuff(ChessMovesPath* cur_Parent)
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
}