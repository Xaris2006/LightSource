#include "chess_entry.h"

#include "chess_blocks.h"

#include "math_Lib.h"

#include <map>
#include <vector>
#include <fstream>

namespace chess
{
	static std::string pci_to_name = "  NBRQK";
	static std::string pci_to_name_small = "  nbrqk";
	
	//maps
	static thread_local std::map<std::string, FileFormat>					file_format;
	static thread_local std::map<char, chess_core::piece_type_identifier>    names_to_pti;

	//chess_entry

	chess_entry::chess_entry()
	{
		
	}

	chess_entry::chess_entry(Pgn_Game& pgngame)
		:m_settings(&pgngame)
	{
		
	}

	bool chess_entry::IsPawnWaiting() const
	{
		return m_game.Is_pawn_waiting();
	}

	void chess_entry::SetNewPawnType(chess_core::piece_type_identifier pti)
	{
		if (m_game.create_new_piece(pti))
		{
			m_waiting_for_pawn_move.n_pieceType = pti;
			new_move(m_waiting_for_pawn_move, false, false);
			m_waiting_for_pawn_move = PropOfMoves();
			m_game.get_player_color() = !m_game.get_player_color();
			number_moves += 1;
		}
	}

	int chess_entry::get_blockID(vec2<float> pos)
	{
		if (m_game.get_blocks(pos).get_class_number())
			return m_game.get_blocks(pos).get_class_number() + 6 * !m_game.get_blocks(pos).get_object_color();
		else
			return 0;
	}

	std::vector<int> chess_entry::get_boardID()
	{
		std::vector<int> output;
		for (int z = 0; z < 8; z++)
		{
			for (int i = 0; i < 8; i++)
			{
				output.push_back(m_game.get_blocks(z, i).get_class_number());
			}
		}
		return output;
	}

	Pgn_Game& chess_entry::GetPgnGame()
	{
		return *m_settings;
	}

	std::string chess_entry::GetECO(const std::string& path) const
	{
		std::ifstream ECOfile(path, std::ios::binary);
		if (ECOfile.is_open())
		{
			Pgn_File pgn;
			ECOfile >> pgn;

			std::vector<int> resaults;
			std::vector<std::string> moves;
			int searched;

			if (m_settings->GetMovePathbyRef().move.size() > 5)
			{
				std::vector<std::string> copy = m_settings->GetMovePathbyCopy().move;
				for (int i = 0; i < 5; i++)
					moves.push_back(copy[i]);
			}
			else
				moves = m_settings->GetMovePathbyCopy().move;

			pgn.SearchMoves(resaults, searched, moves);

			for (int i = 0; i < resaults.size(); i++)
				std::cout << pgn[resaults[i]]["ECO"] << '\n';

			if (resaults.size())
				return pgn[resaults[0]]["ECO"];
			else
				return "?";
		}
		return "?";
	}

	std::string& chess_entry::GetNote(std::vector<int> path)
	{
		if (path.size() == 1 && path[0] == -1)
			return m_settings->GetFirstNote();

		Pgn_Game::ChessMovesPath* copy_path_ptr = &m_settings->GetMovePathbyRef();
		for (int i = 0; i < path.size() - 1; i++)
		{
			if (i % 2 == 1)
			{
				if (copy_path_ptr->children.size() - path[i] <= 0) { return /*risky*/std::string(); }
				auto copy_children_ptr = &copy_path_ptr->children[path[i]];
				copy_path_ptr = copy_children_ptr;
			}
		}
		return copy_path_ptr->details[path[path.size() - 1]];
	}

	Pgn_Game::ChessMovesPath chess_entry::GetMovesByStr() const
	{
		return m_settings->GetMovePathbyCopy();
	}

	std::vector<int> chess_entry::GetMovePath() const
	{
		return m_pmppath;
	}

	PropOfMovesPath chess_entry::GetMovesByData() const
	{
		return m_movespath;
	}

	std::string chess_entry::GetFEN() const
	{
		return m_game.get_FEN();
	}

	std::vector<uint8_t> chess_entry::GetFormatedCurrentPosition() const
	{
		return m_game.get_formated_position();
	}

	bool chess_entry::GetPlayerColor()
	{
		return m_game.get_player_color();
	}

	std::string chess_entry::GetMoveByUCI(const std::string& uciMove)
	{
		std::string strx = "abcdefgh", stry = "12345678";
		vec2<float> pos, dir;
		chess_core::piece_type_identifier type;

		std::string output = "";

		pos.x() = strx.find(uciMove[0]);
		pos.y() = stry.find(uciMove[1]);
		dir.x() = strx.find(uciMove[2]);
		dir.y() = stry.find(uciMove[3]);

		type = m_game.get_blocks(pos).get_class_number();
		if (type == chess_core::KING)
		{
			float movex = dir.x() - pos.x();
			if (movex == 2)
			{
				output = "O-O";
				return output;
			}
			if (movex == -2)
			{
				output = "O-O-O";
				return output;
			}
		}
		if (type == chess_core::PAWN)
			output += uciMove[0];
		else
			output += pci_to_name[type];

		bool showx = false;
		bool showy = false;
		if (type != chess_core::PAWN)
		{
			auto pieces = m_game.get_ptr_piece(type);
			for (auto& piece : pieces)
			{
				if (piece->get_color() != m_game.get_blocks(pos).get_object_color() || piece->get_current_position() == pos)
					continue;
				for (auto& move : piece->possible_moves())
				{
					if (piece->get_current_position() + move == dir)
					{
						if (piece->get_current_position().x() != pos.x())
							showx = true;
						else if (piece->get_current_position().y() != pos.y())
							showy = true;
						break;
					}
				}
				if (showx && showy)
					break;
			}
		}

		if (showx)
			output += uciMove[0];
		if (showy)
			output += uciMove[1];

		if (m_game.get_blocks(dir).get_class_number() != chess_core::UNKOWN)
			output += 'x';
		else if (type == chess_core::PAWN)
		{
			if (dir.x() - pos.x() == 0)
				output = "";
			else
				output += 'x';
		}

		output += uciMove[2];
		output += uciMove[3];

		if (uciMove.size() > 4)
		{
			output += '=';
			output += pci_to_name[pci_to_name_small.find(uciMove[4])];
		}
		return output;
	}

	std::vector<vec2<float>> chess_entry::GetPossibleMoves(const vec2<float>& pos)
	{
		if (m_game.get_blocks(pos).get_object_color() != m_game.get_player_color())
			return std::vector<vec2<float>>();
		return m_game.get_blocks(pos).contains<chess_core::piece>()->possible_moves();
	}

	void chess_entry::run()
	{
		file_format[".chess_core"] = { FileFormat::CHESS };
		file_format[".pgn"] = { FileFormat::PGN };

		names_to_pti['\n'] = { chess_core::piece_type_identifier::PAWN };
		names_to_pti['N'] = { chess_core::piece_type_identifier::KNIGHT };
		names_to_pti['B'] = { chess_core::piece_type_identifier::BISHOP };
		names_to_pti['R'] = { chess_core::piece_type_identifier::ROOK };
		names_to_pti['Q'] = { chess_core::piece_type_identifier::QUEEN };
		names_to_pti['K'] = { chess_core::piece_type_identifier::KING };

		m_energeticMoveBack[chess_core::PAWN] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(pos) = (chess_core::pawn*)pieceptr;
			};
		m_energeticMoveBack[chess_core::KNIGHT] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(pos) = (chess_core::knight*)pieceptr;
			};
		m_energeticMoveBack[chess_core::BISHOP] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(pos) = (chess_core::bishop*)pieceptr;
			};
		m_energeticMoveBack[chess_core::ROOK] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(pos) = (chess_core::rook*)pieceptr;
			};
		m_energeticMoveBack[chess_core::QUEEN] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(pos) = (chess_core::queen*)pieceptr;
			};
		m_energeticMoveBack[chess_core::KING] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(pos) = (chess_core::king*)pieceptr;
			};
		m_energeticMoveBack[chess_core::UNKOWN] = [this](vec2<float> pos, chess_core::piece* pieceptr)
			{
				throw std::runtime_error("Unkown energetic type");
			};

		m_passiveMoveBack[chess_core::PAWN] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(dir) = (chess_core::pawn*)pieceptr;
			};
		m_passiveMoveBack[chess_core::KNIGHT] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(dir) = (chess_core::knight*)pieceptr;
			};
		m_passiveMoveBack[chess_core::BISHOP] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(dir) = (chess_core::bishop*)pieceptr;
			};
		m_passiveMoveBack[chess_core::ROOK] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(dir) = (chess_core::rook*)pieceptr;
			};
		m_passiveMoveBack[chess_core::QUEEN] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(dir) = (chess_core::queen*)pieceptr;
			};
		m_passiveMoveBack[chess_core::KING] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{
				m_game.get_blocks(dir) = (chess_core::king*)pieceptr;

			};
		m_passiveMoveBack[chess_core::UNKOWN] = [this](vec2<float> dir, chess_core::piece* pieceptr)
			{

			};

		//set to zero move
		m_pmppath.push_back(-1);

		m_game.set_position((*m_settings)["FEN"]);

		Pgn_Game::ChessMovesPath& movepath_str = m_settings->GetMovePathbyRef();
		PropOfMovesPath* movepath_data = &m_movespath;

		stringpath_to_srcpath(movepath_str, movepath_data);//BUGGY
	}

	void chess_entry::rerun(Pgn_Game& pgngame)
	{
		m_settings = &pgngame;

		m_movespath.children.clear();
		m_movespath.moves.clear();
		m_pmppath.clear();
		m_game.clear();

		//set to zero move
		m_pmppath.push_back(-1);
		
		m_game.set_position((*m_settings)["FEN"]);
		
		Pgn_Game::ChessMovesPath& movepath_str = m_settings->GetMovePathbyRef();
		PropOfMovesPath* movepath_data = &m_movespath;
		
		stringpath_to_srcpath(movepath_str, movepath_data);//BUGGY
	}

	bool chess_entry::make_new_move(vec2<float> piecpos, vec2<float> piecmove)
	{
		if (m_game.get_player_color() == m_game.get_blocks(piecpos).get_object_color() && !m_game.Is_pawn_waiting())
		{
			//getting the values of the move, so can save it
			chess_core::piece_type_identifier enegertic_type, passive_type;

			m_game.get_blocks(piecpos).get_piece_ptr(enegertic_type);
			chess_core::piece* passive_piece = (chess_core::piece*)m_game.get_blocks(piecpos + piecmove).get_piece_ptr(passive_type);
			
			bool energetic_moved = m_game.get_blocks(piecpos).get_moved();
			bool passive_moved = m_game.get_blocks(piecpos + piecmove).get_moved();
			uint8_t fiftymoverule = m_game.get_fifty_move_rule();

			//an pan san
			if (enegertic_type == chess_core::PAWN && std::abs(piecmove.x()) == std::abs(piecmove.y()) && passive_type == chess_core::UNKOWN)
			{
				passive_piece = (chess_core::piece*)m_game.get_blocks((int)piecpos.x() + (int)piecmove.x(), piecpos.y()).get_piece_ptr(passive_type);
				passive_type = chess_core::piece_type_identifier::UNKOWN;
				if (passive_piece) { passive_moved = ((chess_core::pawn*)passive_piece)->get_moved(); }//shpuld always be true
			}

			bool showverpos = false;
			bool showhorpos = false;

			//check for the possible pieces 
			for (auto& pi : m_game.get_ptr_piece(enegertic_type))
			{
				if (enegertic_type == chess_core::PAWN)
					break;

				if (pi->get_color() == m_game.get_player_color())
				{
					auto& pm = pi->possible_moves();
					for (int x = 0; x < pm.size(); x++)
					{
						if (pm[x] + pi->get_current_position() == piecpos + piecmove)
						{
							if (pi->get_current_position().x() != piecpos.x()) { showhorpos = true; }
							else if (pi->get_current_position().y() != piecpos.y()) { showverpos = true; }
							break;
						}
					}
				}
			}

			//move
			bool valid_move = MoveObject(piecpos, piecmove);

			if (valid_move)
			{
				//checking if a pawn need a new piece
				if (enegertic_type == chess_core::PAWN && (piecpos.y() + piecmove.y() == 0.0f || piecpos.y() + piecmove.y() == 7.0f))
				{
					//m_waiting_for_pawn_move = PropOfMoves{ piecpos, piecmove + piecpos, //
					//	pptr, m_game.get_blocks(piecpos + piecmove).contains<chess_core::pawn>(), //
					//	ecn, pcn, p_move, e_move, chess_core::piece_type_identifier::UNKOWN, fiftymoverule };

					m_waiting_for_pawn_move = PropOfMoves{ 
						m_game.get_blocks(piecpos + piecmove).contains<chess_core::pawn>(), chess_core::PAWN, energetic_moved,
						passive_piece, passive_type, passive_moved,
						piecpos, piecpos + piecmove,
						fiftymoverule,
						chess_core::UNKOWN
					};
					//We wait for the input value of the new piece and then we will save the new move
					return true;
				}

				//saving the new move
				auto cur_move_prop = PropOfMoves{
						m_game.get_blocks(piecpos + piecmove).contains<chess_core::pawn>(), enegertic_type, energetic_moved,
						passive_piece, passive_type, passive_moved,
						piecpos, piecpos + piecmove,
						fiftymoverule,
						chess_core::UNKOWN
				}; 
				
				new_move(cur_move_prop, showverpos, showhorpos);

				m_game.get_player_color() = !m_game.get_player_color();
				number_moves += 1;

				return true;
			}

		}

		return false;
	}

	void chess_entry::PromoteMove(std::vector<int> path, VariationMovement movementType, bool up)
	{
		auto oldpathmove = m_pmppath;
		Go_Start_Position();

		PropOfMovesPath* ptrmovepath = &m_movespath;
		Pgn_Game::ChessMovesPath* ptrpgnmovepath = &m_settings->GetMovePathbyRef();
		int index = 0;
		int moveindex = 0;
		for (int i = 1; i < path.size(); i += 2)
		{
			ptrmovepath = &ptrmovepath->children[path[i]];
			ptrpgnmovepath = &ptrpgnmovepath->children[path[i]];
			index = path[i];
			moveindex = path[i - 1];
		}

		PropOfMovesPath* parent = ptrmovepath->father;
		Pgn_Game::ChessMovesPath* pgnparent = ptrpgnmovepath->parent;
		auto movepath = *ptrmovepath;
		auto pgnmovepath = *ptrpgnmovepath;

		switch (movementType)
		{
		case chess::SWAP:
		{
			if (up)
			{
				if (!parent)
					break;

				int previousMainMove = 0, nextMainMove = 0, indexDepromotedChildren = 0;
				for (int i = path[path.size() - 3] - 1; i > -1; i--)
					if (parent->moves[i].en_type != chess_core::UNKOWN)
					{
						previousMainMove = i;
						break;
					}
				for (int i = path[path.size() - 3] + 1; i < parent->moves.size(); i++)
					if (parent->moves[i].en_type != chess_core::UNKOWN)
					{
						nextMainMove = i;
						break;
					}
				if (!nextMainMove)
					nextMainMove = parent->moves.size();
				
				indexDepromotedChildren = path[path.size() - 2] - path[path.size() - 3] + nextMainMove - 1;
				
				{
					PropOfMovesPath depromoted;
					depromoted.moves.push_back(parent->moves[previousMainMove]);
					
					for (int i = nextMainMove; i < parent->moves.size(); i++)
							depromoted.moves.push_back(parent->moves[i]);
					
					depromoted.children.reserve(parent->children.size() - indexDepromotedChildren);
					for (int i = indexDepromotedChildren; i < parent->children.size(); i++)
						depromoted.children.emplace_back(parent->children[i]);

					parent->moves.resize(nextMainMove);
					parent->children.resize(indexDepromotedChildren + 1);

					parent->moves[previousMainMove] = movepath.moves[0];
					for (int i = 1; i < movepath.moves.size(); i++)
						parent->moves.push_back(movepath.moves[i]);
					

					parent->children.reserve(parent->children.size() + movepath.children.size());
					
					parent->children[path[path.size() - 2]] = depromoted;
					
					for (int i = 0; i < movepath.children.size(); i++)
					{
						auto child = parent->children.emplace_back(movepath.children[i]);
					}
					
					parent->ReloadChildren();
					//parent->children[path[path.size() - 2]].father = parent;
					//for (int i = 0; i < parent->children[path[path.size() - 2]].children.size(); i++)
					//	parent->children[path[path.size() - 2]].children[i].father = &parent->children[path[path.size() - 2]];
				}
				{
					Pgn_Game::ChessMovesPath pgndepromoted;
					pgndepromoted.move.push_back(pgnparent->move[previousMainMove]);
					
					for (int i = nextMainMove; i < pgnparent->move.size(); i++)
						pgndepromoted.move.push_back(pgnparent->move[i]);
				
					pgndepromoted.children.reserve(pgnparent->children.size() - indexDepromotedChildren);
					for (int i = indexDepromotedChildren; i < pgnparent->children.size(); i++)
						pgndepromoted.children.emplace_back(pgnparent->children[i]);

					pgnparent->move.resize(nextMainMove);
					pgnparent->children.resize(indexDepromotedChildren + 1);

					pgnparent->move[previousMainMove] = pgnmovepath.move[0];
					for (int i = 1; i < pgnmovepath.move.size(); i++)
						pgnparent->move.push_back(pgnmovepath.move[i]);

					pgnparent->children.reserve(pgnparent->children.size() + pgnmovepath.children.size());
					
					pgnparent->children[path[path.size() - 2]] = pgndepromoted;
					
					for (int i = 0; i < pgnmovepath.children.size(); i++)
					{
						auto child = pgnparent->children.emplace_back(pgnmovepath.children[i]);
					}
					pgnparent->ReloadChildren();

					//pgnparent->children[path[path.size() - 2]].parent = pgnparent;
					//for (int i = 0; i < pgnparent->children[path[path.size() - 2]].children.size(); i++)
					//	pgnparent->children[path[path.size() - 2]].children[i].parent = &pgnparent->children[path[path.size() - 2]];
				}
			}
			break;
		}
		case chess::END:
		{
			if (up)
			{

			}
			break;
		}
		case chess::INSIDE:
		{	
			if (up)
			{
				if (index && parent->moves[moveindex - 1].en_type == chess_core::UNKOWN)
				{
					PropOfMovesPath copy = parent->children[index - 1];
					parent->children[index - 1] = *ptrmovepath;
					parent->children[index] = copy;
					
					Pgn_Game::ChessMovesPath pgncopy = pgnparent->children[index - 1];
					pgnparent->children[index - 1] = *ptrpgnmovepath;
					pgnparent->children[index] = pgncopy;
				}
			}
			else
			{
				if (parent->moves.size() > moveindex + 1 && parent->moves[moveindex + 1].en_type == chess_core::UNKOWN)
				{
					PropOfMovesPath copy = parent->children[index + 1];
					parent->children[index + 1] = *ptrmovepath;
					parent->children[index] = copy;

					Pgn_Game::ChessMovesPath pgncopy = pgnparent->children[index + 1];
					pgnparent->children[index + 1] = *ptrpgnmovepath;
					pgnparent->children[index] = pgncopy;
				}
			}
			break;
		}
		case chess::INSIDE_MAX:
		{
			if (up)
			{
				while (index && parent->moves[moveindex - 1].en_type == chess_core::UNKOWN)
				{
					PropOfMovesPath copy = parent->children[index - 1];
					parent->children[index - 1] = *ptrmovepath;
					ptrmovepath = &parent->children[index - 1];
					parent->children[index] = copy;

					Pgn_Game::ChessMovesPath pgncopy = pgnparent->children[index - 1];
					pgnparent->children[index - 1] = *ptrpgnmovepath;
					ptrpgnmovepath = &pgnparent->children[index - 1];
					pgnparent->children[index] = pgncopy;

					index -= 1;
					moveindex -= 1;
				}
			}
			else
			{
				while (parent->moves.size() > moveindex + 1 && parent->moves[moveindex + 1].en_type == chess_core::UNKOWN)
				{
					PropOfMovesPath copy = parent->children[index + 1];
					parent->children[index + 1] = *ptrmovepath;
					ptrmovepath = &parent->children[index + 1];
					parent->children[index] = copy;

					Pgn_Game::ChessMovesPath pgncopy = pgnparent->children[index + 1];
					pgnparent->children[index + 1] = *ptrpgnmovepath;
					ptrpgnmovepath = &pgnparent->children[index + 1];
					pgnparent->children[index] = pgncopy;

					index += 1;
					moveindex += 1;
				}
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}

	void chess_entry::DeleteMove(std::vector<int> path)
	{
		if (path.empty())
			return;

		auto oldpathmove = m_pmppath;
		Go_Start_Position();
		

		PropOfMovesPath* ptrmovepath = &m_movespath;
		Pgn_Game::ChessMovesPath* ptrpgnmovepath = &m_settings->GetMovePathbyRef();
		for (int i = 1; i < path.size(); i += 2)
		{
			ptrmovepath = &ptrmovepath->children[path[i]];
			ptrpgnmovepath = &ptrpgnmovepath->children[path[i]];
		}

		if (path[path.size() - 1] == 0 && ptrmovepath->father)
		{
			ptrmovepath = ptrmovepath->father;
			ptrmovepath->children.erase(ptrmovepath->children.begin() + path[path.size() - 2]);
			ptrmovepath->moves.erase(ptrmovepath->moves.begin() + path[path.size() - 3]);
		
			ptrpgnmovepath = ptrpgnmovepath->parent;
			ptrpgnmovepath->children.erase(ptrpgnmovepath->children.begin() + path[path.size() - 2]);
			ptrpgnmovepath->move.erase(ptrpgnmovepath->move.begin() + path[path.size() - 3]);
			
			//Go_next_move_by_path(oldpathmove);
			return;
		}

		ptrmovepath->moves.resize(path[path.size() - 1]);
		ptrpgnmovepath->move.resize(path[path.size() - 1]);

		int childAmount = 0;
		for (auto& move : ptrpgnmovepath->move)
			if (move == "child")
				childAmount += 1;
		ptrmovepath->children.resize(childAmount);
		ptrpgnmovepath->children.resize(childAmount);

		//Go_next_move_by_path(oldpathmove);
	}

	void chess_entry::Go_move_Next()
	{
		if (m_game.Is_pawn_waiting()) { return; }
		PropOfMovesPath* cur_movepath = GetCurMovePath();
		
		//getting the next move value
		if (m_pmppath[m_pmppath.size() - 1] + 1 >= cur_movepath->moves.size()) { return; }
		int privmovenump = m_pmppath[m_pmppath.size() - 1];
		findnextmove(cur_movepath);
		if (privmovenump == m_pmppath[m_pmppath.size() - 1]) { return; }

		PropOfMoves* cur_move = &cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]];

		//getting the current values, so can save it
		cur_move->pa_piece = (chess_core::piece*)m_game.get_blocks(cur_move->dir).get_piece_ptr((chess_core::piece_type_identifier&)cur_move->pa_type);
		if (cur_move->pa_piece) { cur_move->pa_moved = cur_move->pa_piece->get_moved(); }
		cur_move->en_piece = m_game.get_blocks(cur_move->pos).contains<chess_core::piece>();
		cur_move->en_moved = cur_move->en_piece->get_moved();
		
		cur_move->fiftyMoveRule = m_game.get_fifty_move_rule();

		//an pan san
		if (cur_move->en_type == chess_core::PAWN && std::abs(cur_move->dir.x() - cur_move->pos.x()) == std::abs(cur_move->dir.y() - cur_move->pos.y()) && cur_move->pa_type == chess_core::UNKOWN)
		{
			cur_move->pa_piece = (chess_core::piece*)m_game.get_blocks((int)cur_move->dir.x(), (int)cur_move->pos.y()).get_piece_ptr((chess_core::piece_type_identifier&)cur_move->pa_type);
			cur_move->pa_type = chess_core::UNKOWN;
			cur_move->pa_moved = ((chess_core::pawn*)cur_move->pa_piece)->get_moved();
		}

		//check if the move need a new piece value 
		bool pawn_adult = cur_move->en_type == chess_core::piece_type_identifier::PAWN && (cur_move->dir.y() == 0.0f || cur_move->dir.y() == 7.0f);

		vec2<float> move = cur_move->dir - cur_move->pos;

		//move
		cur_move->en_piece->move(cur_move->dir);

		//create a new piece
		if (pawn_adult) { m_game.create_new_piece(cur_move->n_pieceType); }

		m_game.get_player_color() = !m_game.get_player_color();
		number_moves += 1;
	}

	void chess_entry::Go_move_Back()
	{
		if (number_moves == 0) { return; }

		//get the last move
		PropOfMovesPath* cur_movepath = GetCurMovePath();
		PropOfMoves cur_move = cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]];

		//checking if the last move was created a new piece
		if (m_game.Is_pawn_waiting())
		{
			cur_move = m_waiting_for_pawn_move;
			m_waiting_for_pawn_move = PropOfMoves();
		}
		if (cur_move.n_pieceType)
		{
			//delete the new piece
			m_game.delete_last_piece((chess_core::piece_type_identifier)cur_move.n_pieceType);
		}

		m_energeticMoveBack[cur_move.en_type](cur_move.pos, cur_move.en_piece);
		cur_move.en_piece->get_current_position() = cur_move.pos;
		cur_move.en_piece->set_moved(cur_move.en_moved);


		//move the piece that had been eaten
		m_passiveMoveBack[cur_move.pa_type](cur_move.dir, cur_move.pa_piece);
		if(cur_move.pa_type == chess_core::UNKOWN)
		{
			m_game.get_blocks(cur_move.dir) = NULL;
			//un pan san
			if (cur_move.pa_piece)
			{
				m_game.get_blocks((int)cur_move.dir.x(), (int)cur_move.pos.y()) = (chess_core::pawn*)cur_move.pa_piece;
			}
			//roke
			if (cur_move.en_type == chess_core::KING && std::abs(cur_move.dir.x() - cur_move.pos.x()) == 2)
			{
				m_game.get_blocks(mtcs::get_closer(0, 7, (int)cur_move.dir.x()), cur_move.dir.y()) = m_game.get_blocks(mtcs::get_closer(3, 5, (int)cur_move.dir.x()), cur_move.dir.y()).contains<chess_core::rook>();
				m_game.get_blocks(mtcs::get_closer(3, 5, (int)cur_move.dir.x()), cur_move.dir.y()) = NULL;
				m_game.get_blocks(mtcs::get_closer(0, 7, (int)cur_move.dir.x()), cur_move.dir.y()).contains<chess_core::rook>()->get_current_position() = vec2<float>(mtcs::get_closer(0, 7, (int)cur_move.dir.x()), cur_move.dir.y());
				m_game.get_blocks(mtcs::get_closer(0, 7, (int)cur_move.dir.x()), cur_move.dir.y()).contains<chess_core::rook>()->set_moved(false);
			}
		}
		if (cur_move.pa_piece)
		{
			cur_move.pa_piece->set_moved(cur_move.pa_moved);
		}

		if (m_game.Is_pawn_waiting())
		{
			m_game.stop_waiting();
			return;
		}

		m_game.get_fifty_move_rule() = cur_move.fiftyMoveRule;
		if (m_game.get_blocks(cur_move.pos).get_object_color() == chess_core::player_color::BLACK)
			m_game.get_black_moves() -= 1;

		//go one move back
		findprevmove(cur_movepath);

		//set the move for the FEN system
		if (m_pmppath[m_pmppath.size() - 1] + 1
			&& cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]].en_type == chess_core::piece_type_identifier::PAWN
			&& mtcs::absolute(cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]].dir.y() - cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]].pos.y()) == 2.0f)
		{
			m_game.set_last_move_dir(cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]].dir);
		}
		else
			m_game.set_last_move_dir(vec2<float>(0, 0));

		m_game.get_player_color() = !m_game.get_player_color();
		number_moves -= 1;
	}

	void chess_entry::Go_Start_Position()
	{
		while (m_pmppath.size() != 1 || m_pmppath[0] != -1)
		{
			Go_move_Back();
		}
	}

	void chess_entry::Go_next_move_by_path(std::vector<int>& path)
	{
		if (path[0] == -1)
		{
			Go_Start_Position();
			return;
		}
		for (int i = 0; i < path.size(); i++)
		{
			if (i % 2 == 1)
			{
				m_pmppath[m_pmppath.size() - 1] = path[i - 1];
				m_pmppath.push_back(path[i]);
				m_pmppath.push_back(-1);
			}
			else
			{
				auto cur_move_path = GetCurMovePath();
				int not_child_moves = 0;
				for (int j = path[i]; j >= 0; j--)
				{
					if (cur_move_path->moves[j].en_type != chess_core::UNKOWN)
					{
						not_child_moves += 1;
					}
				}
				if (cur_move_path->moves[path[i]].en_type == chess_core::UNKOWN) { not_child_moves -= 1; }
				for (int j = 0; j < not_child_moves; j++)
				{
					Go_move_Next();
				}
			}
		}
	}

	void chess_entry::Go_move_by_str(const std::string& movename)
	{
		chess_core::piece_type_identifier energeticType;
		vec2<float> pos = { 0.0f, 0.0f }, dir = { 0.0f, 0.0f };
		chess_core::piece_type_identifier new_piece_type = chess_core::piece_type_identifier::UNKOWN;

		size_t dir_str_start = 1;

		if (movename[0] == '0' || movename[0] == 'O')
		{
			/*Roke*/
			vec2<float> move;
			if (movename.size() >= 5)
				move = { -2, 0 };
			else if (movename.size() < 5)
				move = { 2, 0 };
			int lane = 0;
			if (!m_game.get_player_color())
				lane = 7;
			pos = vec2<float>(4, lane);//black king

			make_new_move(pos, move);
			return;
		}
		else if (movename[0] >= 'a') { energeticType = chess_core::piece_type_identifier::PAWN; }
		else { energeticType = names_to_pti[movename[0]]; }

		while (movename[dir_str_start] > '8' || (movename[dir_str_start - 1] >= 'A'
			&& movename.size() - (dir_str_start + 1) && movename[dir_str_start + 1] >= 'a'
			&& energeticType != chess_core::piece_type_identifier::PAWN))
		{
			dir_str_start += 1;
		}

		dir = vec2<float>(float((int)movename[dir_str_start - 1] - (int)'a'), float((int)movename[dir_str_start] - (int)'0' - 1));
		if (movename.size() - (dir_str_start) > 1)
		{
			if (movename[dir_str_start + 1] == '=')
			{
				std::string piecemames = "  NBRQ";
				new_piece_type = (chess_core::piece_type_identifier)piecemames.find(movename[dir_str_start + 2]);
			}
		}


		//check for the posible pieces 
		auto pieceptr = m_game.get_ptr_piece(energeticType);
		bool color = m_game.get_player_color();
		std::vector<vec2<float>> posiblepos;


		for (size_t j = 0; j < pieceptr.size(); j++)
		{
			if (pieceptr[j]->get_color() == color)
			{
				auto& pm = pieceptr[j]->possible_moves();
				for (int x = 0; x < pm.size(); x++)
				{
					if (pm[x] + pieceptr[j]->get_current_position() == dir) { posiblepos.push_back(pieceptr[j]->get_current_position()); }
				}
			}
		}
		if (posiblepos.size() != 1)
		{

			if (movename[1] >= 'a' && energeticType != chess_core::PAWN)
			{
				for (size_t j = 0; j < posiblepos.size(); j++)
				{
					if (posiblepos[j].x() == (int)movename[1] - (int)'a')
					{
						if (movename[2] <= '8')
						{
							//0    1    2    3    4
							//name xpos ypos xdir ydir
							if (posiblepos[j].y() == (int)movename[2] - (int)'0' - 1) { pos = posiblepos[j]; break; }
						}

						//0    1    2    3
						//name xpos xdir ydir
						pos = posiblepos[j]; break;
					}
				}
			}
			else if (movename[1] <= '8' && energeticType != chess_core::PAWN)
			{
				for (size_t j = 0; j < posiblepos.size(); j++)
				{
					//0    1    2    3
					//name ypos xdir ydir
					if (posiblepos[j].y() == (int)movename[1] - (int)'0' - 1) { pos = posiblepos[j]; break; }
				}
			}
			else
			{
				for (size_t j = 0; j < posiblepos.size(); j++)
				{
					//0    2    3
					//name xdir ydir
					if (posiblepos[j].x() == (int)movename[0] - (int)'a') { pos = posiblepos[j]; break; }
				}
			}
		}
		else
			pos = posiblepos[0];
		make_new_move(pos, dir - pos);
	}

	bool chess_entry::MoveObject(vec2<float>& pos, vec2<float>& end_move)
	{
		if (m_game.ERROR_EXIT) { return false; }

		std::vector<vec2<float>>& pm = m_game.get_blocks(pos).contains<chess_core::piece>()->possible_moves();

		for (int z = 0; z < pm.size(); z++)
		{
			if (pm[z] == end_move)
			{
				if (m_game.get_blocks(pos).contains<chess_core::piece>()->move(pos + end_move))
					return true;
				break;
			}
		}
		return false;
	}

	bool chess_entry::IsFileLoaded() const
	{
		if (&m_settings)
			return true;
		return false;
	}

	void chess_entry::stringpath_to_srcpath(Pgn_Game::ChessMovesPath& cmp, PropOfMovesPath* pmp)
	{
		size_t moves_next = 0;
		for (size_t i = 0; i < cmp.move.size(); i++)
		{
			pmp->moves.push_back(PropOfMoves());
			if (cmp.move[i] == "child")
			{
				Go_move_Back();

				m_pmppath[m_pmppath.size() - 1] = pmp->moves.size() - 1;
				m_pmppath.push_back(pmp->children.size());
				m_pmppath.push_back(-1);

				pmp->children.push_back(PropOfMovesPath(pmp));
				pmp->moves[pmp->moves.size() - 1].en_type = chess_core::piece_type_identifier::UNKOWN;
				stringpath_to_srcpath(cmp.children[pmp->children.size() - 1], &pmp->children[pmp->children.size() - 1]);

				Go_move_Next();
				continue;
			}

			chess_core::piece_type_identifier ecn;
			vec2<float> pos = { 0.0f, 0.0f }, dir = { 0.0f, 0.0f };
			chess_core::piece_type_identifier new_piece_type = chess_core::piece_type_identifier::UNKOWN;

			size_t move_without_num = 0;
			size_t dir_str_start = 1;

			//0      1 2 3 4 
			//number .   move
			//move	
			if (cmp.move[i][0] != '0')
			{
				for (int j = 0; j < cmp.move[i].size(); j++)
				{
					if (cmp.move[i][j] <= '9' && cmp.move[i][j] >= '0') { move_without_num += 1; }
					else if (cmp.move[i][j] == '.') { move_without_num += 2; break; }
					else { break; }
				}
			}
			if (cmp.move[i][move_without_num] == '0' || cmp.move[i][move_without_num] == 'O')
			{
				/*Roke*/
				vec2<float> move;
				if (cmp.move[i].size() - move_without_num >= 5)
					move = { -2, 0 };
				else if (cmp.move[i].size() - move_without_num < 5)
					move = { 2, 0 };

				if (move_without_num)
				{
					pmp->moves[pmp->moves.size() - 1].pos = vec2<float>(4, 0);//white king
					pmp->moves[pmp->moves.size() - 1].dir = pmp->moves[pmp->moves.size() - 1].pos + move;
					pmp->moves[pmp->moves.size() - 1].en_type = chess_core::piece_type_identifier::KING;
				}
				else
				{
					pmp->moves[pmp->moves.size() - 1].pos = vec2<float>(4, 7);//black king
					pmp->moves[pmp->moves.size() - 1].dir = pmp->moves[pmp->moves.size() - 1].pos + move;
					pmp->moves[pmp->moves.size() - 1].en_type = chess_core::piece_type_identifier::KING;
				}
				moves_next += 1;
				Go_move_Next();
				continue;
			}
			else if (cmp.move[i][move_without_num] >= 'a') { ecn = chess_core::piece_type_identifier::PAWN; }
			else { ecn = names_to_pti[cmp.move[i][move_without_num]]; }

			while (cmp.move[i][move_without_num + dir_str_start] > '8' || (cmp.move[i][move_without_num + dir_str_start - 1] >= 'A'
				&& cmp.move[i].size() - (move_without_num + dir_str_start + 1) && cmp.move[i][move_without_num + dir_str_start + 1] >= 'a'
				&& ecn != chess_core::piece_type_identifier::PAWN))
			{
				dir_str_start += 1;
			}

			dir = vec2<float>(float((int)cmp.move[i][move_without_num + dir_str_start - 1] - (int)'a'), float((int)cmp.move[i][move_without_num + dir_str_start] - (int)'0' - 1));
			if (cmp.move[i].size() - (move_without_num + dir_str_start) > 1)
			{
				if (cmp.move[i][move_without_num + dir_str_start + 1] == '=')
				{
					std::string piecemames = "  NBRQ";
					new_piece_type = (chess_core::piece_type_identifier)piecemames.find(cmp.move[i][move_without_num + dir_str_start + 2]);
				}
			}


			//check for the posible pieces 
			auto p = m_game.get_ptr_piece(ecn);
			bool color = m_game.get_player_color();
			std::vector<vec2<float>> ppos;


			for (size_t j = 0; j < p.size(); j++)
			{
				if (p[j]->get_color() == color)
				{
					auto& pm = p[j]->possible_moves();
					for (int x = 0; x < pm.size(); x++)
					{
						if (pm[x] + p[j]->get_current_position() == dir) { ppos.push_back(p[j]->get_current_position()); }
					}
				}
			}
			if (ppos.size() != 1)
			{

				if (cmp.move[i][move_without_num + 1] >= 'a' && ecn != chess_core::PAWN)
				{
					for (size_t j = 0; j < ppos.size(); j++)
					{
						if (ppos[j].x() == (int)cmp.move[i][move_without_num + 1] - (int)'a')
						{
							if (cmp.move[i][move_without_num + 2] <= '8')
							{
								//0    1    2    3    4
								//name xpos ypos xdir ydir
								if (ppos[j].y() == (int)cmp.move[i][move_without_num + 2] - (int)'0' - 1) { pos = ppos[j]; break; }
							}

							//0    1    2    3
							//name xpos xdir ydir
							pos = ppos[j]; break;
						}
					}
				}
				else if (cmp.move[i][move_without_num + 1] <= '8' && ecn != chess_core::PAWN)
				{
					for (size_t j = 0; j < ppos.size(); j++)
					{
						//0    1    2    3
						//name ypos xdir ydir
						if (ppos[j].y() == (int)cmp.move[i][move_without_num + 1] - (int)'0' - 1) { pos = ppos[j]; break; }
					}
				}
				else
				{
					//ecn == pawn
					for (size_t j = 0; j < ppos.size(); j++)
					{
						//0    2    3
						//name xdir ydir
						if (ppos[j].x() == (int)cmp.move[i][move_without_num] - (int)'a') { pos = ppos[j]; break; }
					}
				}
			}
			else
				pos = ppos[0];
			pmp->moves[pmp->moves.size() - 1].pos = pos;
			pmp->moves[pmp->moves.size() - 1].dir = dir;
			pmp->moves[pmp->moves.size() - 1].en_type = ecn;
			pmp->moves[pmp->moves.size() - 1].n_pieceType = new_piece_type;

			moves_next += 1;
			Go_move_Next();
		}

		for (int i = 0; i < moves_next; i++)
		{
			Go_move_Back();
		}
	}

	void chess_entry::findnextmove(PropOfMovesPath*& p)
	{
		for (int i = m_pmppath[m_pmppath.size() - 1] + 1; i < p->moves.size(); i++)
		{
			if (p->moves[i].en_type != chess_core::UNKOWN)
			{
				m_pmppath[m_pmppath.size() - 1] = i;
				return;
			}
		}
	}

	void chess_entry::findprevmove(PropOfMovesPath*& p)
	{
		if (p->moves[m_pmppath[m_pmppath.size() - 1]].en_type == chess_core::UNKOWN)
		{
			for (int i = m_pmppath[m_pmppath.size() - 1] - 1; i > -1; i--)
			{
				if (p->moves[i].en_type != chess_core::UNKOWN)
				{
					m_pmppath[m_pmppath.size() - 1] = i;
					break;
				}
			}
		}
		for (int i = m_pmppath[m_pmppath.size() - 1] - 1; i > -1; i--)
		{
			if (p->moves[i].en_type != chess_core::UNKOWN)
			{
				m_pmppath[m_pmppath.size() - 1] = i;
				return;
			}
		}

		if (!p->father) { m_pmppath[m_pmppath.size() - 1] -= 1; return; }
		p = p->father;
		m_pmppath.resize(m_pmppath.size() - 2);
		findprevmove(p);
	}

	void chess_entry::new_move(PropOfMoves& nm, const bool& showverpos, const bool& showhorpos)
	{
		PropOfMovesPath* cur_movepath = GetCurMovePath();

		if (m_pmppath[m_pmppath.size() - 1] == cur_movepath->LastValidMove())
		{
			cur_movepath->moves.push_back(nm);

			//converting move data to str move

			std::string move = "";
			const char* piecemames = "  NBRQK";
			const char* hor_count = "abcdefgh";
			const char* ver_count = "12345678";
			

			if (m_game.get_player_color() == chess_core::WHITE) { move += mtcs::trans_str(number_moves / 2 + 1) + ". "; }
			if (nm.en_type == chess_core::KING && std::abs(nm.dir.x() - nm.pos.x()) == 2.0f)
			{
				//roke
				if (nm.dir.x() == 6.0f)
					move += "O-O";
				else
					move += "O-O-O";
			}
			else
			{
				if (nm.en_type - 1) { move += piecemames[nm.en_type]; }
				if (nm.en_type == chess_core::PAWN && nm.pa_type != chess_core::UNKOWN) { move += hor_count[(int)(nm.pos.x())]; }
				if (showhorpos) { move += hor_count[(int)(nm.pos.x())]; }
				if (showverpos) { move += ver_count[(int)(nm.pos.y())]; }
				if (nm.pa_type != chess_core::UNKOWN) { move += 'x'; }
				if (nm.en_type == chess_core::PAWN
					&& nm.pa_type == chess_core::UNKOWN
					&& std::abs(nm.dir.x() - nm.pos.x()) == std::abs(nm.dir.y() - nm.pos.y()))
				{
					move += hor_count[(int)(nm.pos.x())];
					move += 'x';
				}
				move += hor_count[(int)(nm.dir.x())];
				move += ver_count[(int)(nm.dir.y())];
			}
			if (nm.n_pieceType != chess_core::UNKOWN) { move += '='; move += piecemames[nm.n_pieceType]; }

			if (m_game.king_hitted(!m_game.get_player_color()))
			{
				if (m_game.check_for_ending(!m_game.get_player_color())) { move += '#'; }
				else { move += '+'; }
			}

			if (m_pmppath.size()) { m_pmppath[m_pmppath.size() - 1] = cur_movepath->LastValidMove(); }

			Pgn_Game::ChessMovesPath* cmp = &m_settings->GetMovePathbyRef();

			for (int i = 0; i < m_pmppath.size(); i++)
			{
				if (i % 2 == 1) { cmp = &cmp->children[m_pmppath[i]]; }
			}
			cmp->move.push_back(move);
		}
		else
		{
			auto movepath = GetCurMovePath();
			findnextmove(movepath);
			if (cur_movepath->moves[m_pmppath[m_pmppath.size() - 1]] == nm)
			{
				return;
			}
			findprevmove(movepath);

			int child_moves_before = 0;
			int child_move_after = 0;

			for (int i = 0; i < m_pmppath[m_pmppath.size() - 1]; i++)
			{
				if (cur_movepath->moves[i].en_type == chess_core::UNKOWN) { child_moves_before += 1; }
			}
			for (int i = m_pmppath[m_pmppath.size() - 1] + 1; i < cur_movepath->moves.size(); i++)
			{
				if (cur_movepath->moves[i].en_type == chess_core::UNKOWN) { child_move_after += 1; }
				else
					break;
			}

			for (int i = child_moves_before; i < child_moves_before + child_move_after; i++)
			{
				if (cur_movepath->children[i].moves[0] == nm)
				{
					m_pmppath[m_pmppath.size() - 1] += (2 + i);
					m_pmppath.push_back(i);
					m_pmppath.push_back(0);
					return;
				}
			}

			findnextmove(cur_movepath);

			int child_order = 0;
			int move_order = cur_movepath->moves.size();

			for (int i = 0; i < m_pmppath[m_pmppath.size() - 1]; i++)
				if (cur_movepath->moves[i].en_type == chess_core::UNKOWN) { child_order += 1; }
			for (int i = m_pmppath[m_pmppath.size() - 1] + 1; i < cur_movepath->moves.size(); i++)
			{
				if (cur_movepath->moves[i].en_type == chess_core::UNKOWN) { child_order += 1; }
				else
					break;
			}

			cur_movepath->children.insert(cur_movepath->children.begin() + child_order, PropOfMovesPath(cur_movepath));
			cur_movepath->children[child_order].moves.push_back(nm);

			cur_movepath->ReloadChildren();

			for (int i = m_pmppath[m_pmppath.size() - 1] + 1; i < cur_movepath->moves.size(); i++)
				if (cur_movepath->moves[i].en_type != chess_core::UNKOWN) { move_order = i; break; }

			cur_movepath->moves.insert(cur_movepath->moves.begin() + move_order, PropOfMoves());

			cur_movepath = &cur_movepath->children[child_order];

			//converting move data to str move

			std::string move = "";
			const char* piecemames = "  NBRQK";
			const char* hor_count = "abcdefgh";
			const char* ver_count = "12345678";

			if (m_game.get_player_color() == chess_core::WHITE) { move += mtcs::trans_str(number_moves / 2 + 1) + ". "; }
			if (nm.en_type == chess_core::KING && std::abs(nm.dir.x() - nm.pos.x()) == 2.0f)
			{
				//roke
				if (nm.dir.x() == 6.0f)
					move += "O-O";
				else
					move += "O-O-O";
			}
			else
			{
				if (nm.en_type - 1) { move += piecemames[nm.en_type]; }
				if (nm.en_type == chess_core::PAWN && nm.pa_type != chess_core::UNKOWN) { move += hor_count[(int)(nm.pos.x())]; }
				if (showhorpos) { move += hor_count[(int)(nm.pos.x())]; }
				if (showverpos) { move += ver_count[(int)(nm.pos.y())]; }
				if (nm.pa_type != chess_core::UNKOWN) { move += 'x'; }
				if (nm.en_type == chess_core::PAWN
					&& nm.pa_type == chess_core::UNKOWN
					&& std::abs(nm.dir.x() - nm.pos.x()) == std::abs(nm.dir.y() - nm.pos.y()))
				{
					move += hor_count[(int)(nm.pos.x())];
					move += 'x';
				}

				move += hor_count[(int)(nm.dir.x())];
				move += ver_count[(int)(nm.dir.y())];
			}
			if (nm.n_pieceType != chess_core::UNKOWN) { move += '='; move += piecemames[nm.n_pieceType]; }

			if (m_game.king_hitted(!m_game.get_player_color()))
			{
				if (m_game.check_for_ending(!m_game.get_player_color())) { move += '#'; }
				else { move += '+'; }
			}

			m_pmppath[m_pmppath.size() - 1] = move_order;
			m_pmppath.push_back(child_order);
			m_pmppath.push_back(0);

			//save str move

			Pgn_Game::ChessMovesPath* cmp = &m_settings->GetMovePathbyRef();

			for (int i = 0; i < m_pmppath.size(); i++)
			{
				if (i == m_pmppath.size() - 2) { cmp->move.insert(cmp->move.begin() + (m_pmppath[m_pmppath.size() - 3]), "child"); cmp->children.insert(cmp->children.begin() + child_order, Pgn_Game::ChessMovesPath(cmp)); }
				if (i % 2 == 1) { cmp = &cmp->children[m_pmppath[i]]; }
			}
			cmp->move.push_back(move);

			cmp->ReloadChildren();

			newVariation = true;
		}
	}

	PropOfMovesPath* chess_entry::GetCurMovePath()
	{
		PropOfMovesPath* output = &m_movespath;
		for (size_t i = 0; i < m_pmppath.size(); i++)
		{
			if (i % 2 == 1) { output = &output->children[m_pmppath[i]]; }
		}
		return output;
	}
}
