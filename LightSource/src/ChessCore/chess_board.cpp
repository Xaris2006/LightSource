#include "chess_board.h"

#include "chess_pecie.h"

#include "math_Lib.h"
#include "macros.h"

#include <map>


#define ERROR_RETURN(message) ERROR_EXIT = true; \
		Error(std::cout << '\n') << message << '\n';\
		return

namespace chess_core
{	
	//structs
	struct KingSecurity
	{
		bool (*func)(board*, piece*, vec2<float>, bool);
	};

	//map
	std::map<int, KingSecurity> kingsecuritymap;

	template<typename passive_object>
	bool king_security(board* game, piece* object, vec2<float> direction, bool real)
	{
		if (!real) { return true; }

		auto block_contains = game->get_blocks(direction).contains<passive_object>();
		bool moved = object->get_moved();
		vec2<float> current_position = object->get_current_position();
		object->move(direction, false, false);
		if (game->king_hitted(object->get_color()))
		{
			game->get_blocks(current_position).set_contained_piece(object, game->get_blocks(direction).get_class_number());
			object->get_current_position() = current_position;
			object->set_moved(moved);
			game->get_blocks(direction) = block_contains;
			return false;
		}
		else
		{
			game->get_blocks(current_position).set_contained_piece(object, game->get_blocks(direction).get_class_number());
			object->get_current_position() = current_position;
			object->set_moved(moved);
			game->get_blocks(direction) = block_contains;
			return true;
		}
	}

	//board 

	board::board()
	{
		ERROR_EXIT = false;
		
		//building the needed types of king_security
		kingsecuritymap[piece_type_identifier::PAWN]   = {  king_security<pawn>   };
		kingsecuritymap[piece_type_identifier::KNIGHT] = {  king_security<knight> };
		kingsecuritymap[piece_type_identifier::BISHOP] = {  king_security<bishop> };
		kingsecuritymap[piece_type_identifier::ROOK]   = {  king_security<rook>   };
		kingsecuritymap[piece_type_identifier::QUEEN]  = {  king_security<queen>  };
		kingsecuritymap[piece_type_identifier::KING]   = {  king_security<king>	  };
		kingsecuritymap[piece_type_identifier::UNKOWN] = {
			[](board* game, piece* object, vec2<float> direction, bool real)->bool
			{ 
				if (!real) { return true; }

				bool unpansan = false;
				bool roke = false;
				pawn* p_pawn = nullptr;
				if (game->get_blocks(object->get_current_position()).get_class_number() == piece_type_identifier::PAWN
					&& mtcs::absolute(direction.x() - object->get_current_position().x()) == mtcs::absolute(direction.y() - object->get_current_position().y()))
				{
					p_pawn = game->get_blocks((int)direction.x(), (int)object->get_current_position().y()).contains<pawn>();
					unpansan = true;
				}
				if (game->get_blocks(object->get_current_position()).get_class_number() == piece_type_identifier::KING
					&& mtcs::absolute(direction.x() - object->get_current_position().x()) == 2.0f)
				{
					roke = true;
				}

				vec2<float> current_position = object->get_current_position();
				bool moved = object->get_moved();
				object->move(direction, false, false);
				if (game->king_hitted(object->get_color()))
				{
					game->get_blocks(current_position).set_contained_piece(object, game->get_blocks(direction).get_class_number());
					object->get_current_position() = current_position;
					object->set_moved(moved);
					if (unpansan) { game->get_blocks((int)direction.x(), (int)current_position.y()) = p_pawn; }
					else if (roke)
					{
						game->get_blocks(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y()) = game->get_blocks(mtcs::get_closer(3, 5, (int)direction.x()), (int)direction.y()).contains<rook>();
						game->get_blocks(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y()).contains<rook>()->get_current_position() = vec2<float>(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y());
						game->get_blocks(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y()).contains<rook>()->set_moved(false);
						game->get_blocks(mtcs::get_closer(3, 5, (int)direction.x()), (int)direction.y()) = NULL;
					}

					game->get_blocks(direction) = NULL;

					return false;
				}
				else
				{
					game->get_blocks(current_position).set_contained_piece(object, game->get_blocks(direction).get_class_number());
					object->get_current_position() = current_position;
					object->set_moved(moved);
					if (unpansan) { game->get_blocks((int)direction.x(), (int)current_position.y()) = p_pawn; }
					else if (roke)
					{
						game->get_blocks(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y()) = game->get_blocks(mtcs::get_closer(3, 5, (int)direction.x()), (int)direction.y()).contains<rook>();
						game->get_blocks(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y()).contains<rook>()->get_current_position() = vec2<float>(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y());
						game->get_blocks(mtcs::get_closer(0, 7, (int)direction.x()), (int)direction.y()).contains<rook>()->set_moved(false);
						game->get_blocks(mtcs::get_closer(3, 5, (int)direction.x()), (int)direction.y()) = NULL;
					}

					game->get_blocks(direction) = NULL;

					return true;
				}
			}
		};
	}

	board::~board()
	{

	}

	void board::PositionERROR()
	{
		int allWhitepecies = 0;
		int allBlackpecies = 0; 
		auto GetAllColors = [&allWhitepecies, &allBlackpecies](std::vector<piece>& pieces)
		{
			for (auto& piece : pieces)
			{
				if (piece.get_color())
					allWhitepecies += 1;
				else
					allBlackpecies += 1;
			}
		};

		auto GetPieceColorsAndAmount = [this](std::vector<piece>& piece, piece_type_identifier pieceType)
		{
			int whitePieces = 0;
			int blackPieces = 0;
			for (auto& piece : piece)
			{
				if (piece.get_color())
					whitePieces += 1;
				else
					blackPieces += 1;

				if (pieceType == PAWN)
				{
					if (piece.get_current_position().y() == 0.0f || piece.get_current_position().y() == 7.0f)
					{
						ERROR_RETURN("One of your pawns is at line 1 or 8");
					}
				}
			}

			switch (pieceType)
			{
			case chess_core::UNKOWN:
				break;
			case chess_core::PAWN:
			{
				if (whitePieces > 8)
				{
					ERROR_RETURN("Too many white pawns");
				}
				if (blackPieces > 8)
				{
					ERROR_RETURN("Too many black pawns");
				}
				break;
			}
			case chess_core::KNIGHT:
			{
				if (whitePieces > 10)
				{
					ERROR_RETURN("Too many white knights");
				}
				if (blackPieces > 10)
				{
					ERROR_RETURN("Too many black knights");
				}
				break;
			}
			case chess_core::BISHOP:
			{
				if (whitePieces > 10)
				{
					ERROR_RETURN("Too many white bishops");
				}
				if (blackPieces > 10)
				{
					ERROR_RETURN("Too many black bishops");
				}
				break;
			}
			case chess_core::ROOK:
			{
				if (whitePieces > 10)
				{
					ERROR_RETURN("Too many white rooks");
				}
				if (blackPieces > 10)
				{
					ERROR_RETURN("Too many black rooks");
				}
				break;
			}
			case chess_core::QUEEN:
			{
				if (whitePieces > 9)
				{
					ERROR_RETURN("Too many white queens");
				}
				if (blackPieces > 9)
				{
					ERROR_RETURN("Too many black queens");
				}
				break;
			}
			case chess_core::KING:
				break;
			default:
				break;
			}
		};

		GetAllColors((std::vector<piece>&)m_pawns);
		GetAllColors((std::vector<piece>&)m_knights);
		GetAllColors((std::vector<piece>&)m_bishops);
		GetAllColors((std::vector<piece>&)m_rooks);
		GetAllColors((std::vector<piece>&)m_queens);
		GetAllColors((std::vector<piece>&)m_kings);

		if (allWhitepecies > 16 || allBlackpecies > 16)
		{
			ERROR_RETURN("Too many piecies");
		}
		
		GetPieceColorsAndAmount((std::vector<piece>&)m_pawns	, PAWN);
		GetPieceColorsAndAmount((std::vector<piece>&)m_knights, KNIGHT);
		GetPieceColorsAndAmount((std::vector<piece>&)m_bishops, BISHOP);
		GetPieceColorsAndAmount((std::vector<piece>&)m_rooks	, ROOK);
		GetPieceColorsAndAmount((std::vector<piece>&)m_queens , QUEEN);

		//king
		if (m_kings.size() != 2)
		{
			ERROR_RETURN("The game did not start with two kings");
		}
		if (m_kings[0].get_color() == m_kings[1].get_color())
		{
			ERROR_RETURN("Kings have the same color");
		}
		else if (mtcs::absolute(m_kings[0].get_current_position().x() - m_kings[1].get_current_position().x()) <= 1.0f
			&& mtcs::absolute(m_kings[0].get_current_position().y() - m_kings[1].get_current_position().y()) <= 1.0f)
		{
			ERROR_RETURN("Your kings are too close to each other");
		}
		
		if (king_hitted(!m_player))
		{
			ERROR_RETURN("Yuor king is checked");
		}

	}

	blocks& board::get_blocks(int x, int y)
	{
		return m_block[x][y];
	}

	blocks& board::get_blocks(vec2<float> position)
	{
		return m_block[(int)position.x()][(int)position.y()];
	}

	vec2<float> board::get_last_move_dir() const
	{
		return m_lastmovedir;
	}

	std::vector<piece*> board::get_ptr_piece(uint8_t cn) const
	{
		std::vector<piece*> rv;

		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				if (m_block[x][y].get_class_number() == cn)
				{
					rv.push_back(m_block[x][y].contains<piece>());
				}
			}
		}
		return rv;
	}

	bool& board::get_player_color()
	{
		return m_player;
	}

	std::string board::get_FEN() const
	{
		if (ERROR_EXIT) { return ""; }

		std::string Fenstr = "";

		std::string piecenames = "pnbrqkPNBRQK";

		for (int y = 7; y > -1; y--)
		{
			char empty = '0';
			for (int x = 0; x < 8; x++)
			{
				if (m_block[x][y].get_class_number() != piece_type_identifier::UNKOWN)
				{
					if (empty != '0') { Fenstr += empty; empty = '0'; }
					Fenstr += piecenames[(m_block[x][y].get_class_number() - 1) + 6 * m_block[x][y].get_object_color()];
				}
				else
					empty += 1;
			}
			if (empty != '0') { Fenstr += empty; }
			if (y != 0) { Fenstr += '/'; }
		}

		Fenstr += ' ';

		if (m_player)
			Fenstr += 'w';
		else
			Fenstr += 'b';

		Fenstr += ' ';

		bool roke = false;

		for (int i = 1; i >= 0; i--)
		{
			if (m_kings[i].get_current_position().x() != 4.0f)
				continue;
			if (!m_kings[i].get_moved()
				&& m_block[(int)m_kings[i].get_current_position().x() + 3][(int)m_kings[i].get_current_position().y()].get_class_number() == piece_type_identifier::ROOK
				&& !m_block[(int)m_kings[i].get_current_position().x() + 3][(int)m_kings[i].get_current_position().y()].get_moved())
			{
				if (m_kings[i].get_color())
				{
					Fenstr += 'K';
					roke = true;
				}
				else
				{
					Fenstr += 'k';
					roke = true;
				}
			}

			if (!m_kings[i].get_moved()
				&& m_block[(int)m_kings[i].get_current_position().x() - 4][(int)m_kings[i].get_current_position().y()].get_class_number() == piece_type_identifier::ROOK
				&& !m_block[(int)m_kings[i].get_current_position().x() - 4][(int)m_kings[i].get_current_position().y()].get_moved())
			{
				if (m_kings[i].get_color())
				{
					Fenstr += 'Q';
					roke = true;
				}
				else
				{
					Fenstr += 'q';
					roke = true;
				}
			}
		}
		if (!roke) { Fenstr += '-'; }

		Fenstr += ' ';
		auto last_pawn_move = m_lastmovedir;
		if (last_pawn_move == vec2<float>(0, 0))
		{
			Fenstr += '-'; 
		}
		else
		{
			Fenstr += 'a' + (int)last_pawn_move.x();
			if ((int)last_pawn_move.y() == 4) { Fenstr += '6'; }
			else
				Fenstr += '3';
		}

		Fenstr += (' ' + mtcs::trans_str(m_FiftyMoveRule));
		Fenstr += (' ' + mtcs::trans_str(m_BlackMoves));

		return Fenstr;
	}

	std::vector<uint8_t> board::get_formated_position() const
	{
		if (ERROR_EXIT) { return std::vector<uint8_t>(); }

		std::vector<uint8_t> pos;
		int blockFilled = 0;
		bool firstfour = true;
		int blocks_empty = 0;

		uint8_t classnames[6] = { 0b00000000, 0b00000001, 0b00000010,
								  0b00000011, 0b00000100, 0b00000101 };
		uint8_t color[2] = { 0b00000000, 0b00001000 };
		uint8_t empty[4] = { 0b00000110, 0b00001110, 0b00000111, 0b00001111 };

		pos.push_back(0b00000000);
		for (int y = 7; y >= 0; y--)
		{
			for (int x = 0; x < 8; x++)
			{
				if (blockFilled == 8)
				{
					blockFilled = 0;
					pos.push_back(0b00000000);
				}
				if (m_block[x][y].get_class_number() == UNKOWN)
				{
					blocks_empty += 1;
					if (blocks_empty == 4)
					{
						size_t s = pos.size();
						pos[s - 1] = pos[s - 1] | empty[blocks_empty - 1];
						if (firstfour)
						{	
							pos[s - 1] = pos[s - 1] << 4;
							firstfour = false;
						}
						else
							firstfour = true;
						blockFilled += 4;
						blocks_empty = 0;
					}
					continue;
				}
				else if (blocks_empty)
				{
					size_t s = pos.size();
					pos[s - 1] = pos[s - 1] | empty[blocks_empty - 1];
					if (firstfour)
					{
						pos[s - 1] = pos[s - 1] << 4; 
						firstfour = false;
					}
					else
						firstfour = true;
					blockFilled += 4;
					blocks_empty = 0;
					if (blockFilled == 8)
					{
						blockFilled = 0;
						pos.push_back(0b00000000);
					}
				}
				size_t s = pos.size();
				pos[s - 1] = pos[s - 1] | classnames[m_block[x][y].get_class_number() - 1];
				pos[s - 1] = pos[s - 1] | color[m_block[x][y].get_object_color()];
				if (firstfour)
				{
					pos[s - 1] = pos[s - 1] << 4;
					firstfour = false;
				}
				else
					firstfour = true;
				blockFilled += 4;
			}

		}
		if (blocks_empty)
		{
			size_t s = pos.size();
			pos[s - 1] = pos[s - 1] | empty[blocks_empty - 1];
			if (firstfour)
				pos[s - 1] = pos[s - 1] << 4;
		}
		pos.push_back(((uint8_t)m_lastmovedir.x() | ((uint8_t)m_player << 3)) << 4);
		size_t s = pos.size();
		for (int i = 1; i >= 0; i--)
		{
			if (m_kings[i].get_current_position().x() != 4.0f)
				continue;
			if (!m_kings[i].get_moved()
				&& m_block[(int)m_kings[i].get_current_position().x() + 3][(int)m_kings[i].get_current_position().y()].get_class_number() == piece_type_identifier::ROOK
				&& !m_block[(int)m_kings[i].get_current_position().x() + 3][(int)m_kings[i].get_current_position().y()].get_moved())
			{
				if (m_kings[i].get_color())
					pos[s - 1] = pos[s - 1] | 0b00000001;
				else
					pos[s - 1] = pos[s - 1] | 0b00000100;
			}

			if (!m_kings[i].get_moved()
				&& m_block[(int)m_kings[i].get_current_position().x() - 4][(int)m_kings[i].get_current_position().y()].get_class_number() == piece_type_identifier::ROOK
				&& !m_block[(int)m_kings[i].get_current_position().x() - 4][(int)m_kings[i].get_current_position().y()].get_moved())
			{
				if (m_kings[i].get_color())
					pos[s - 1] = pos[s - 1] | 0b00000010;
				else
					pos[s - 1] = pos[s - 1] | 0b00001000;
			}
		}
		return pos;
	}

	unsigned int& board::get_fifty_move_rule()
	{
		return m_FiftyMoveRule;
	}
	
	unsigned int& board::get_black_moves()
	{
		return m_BlackMoves;
	}

	void board::set_last_move_dir(vec2<float> last_move_dir)
	{
		m_lastmovedir = last_move_dir;
	}

	bool board::Is_pawn_waiting() const
	{
		if (m_almost_adult)
			return true;
		else
			return false;
	}

	void board::stop_waiting()
	{
		m_almost_adult = nullptr;
	}

	void board::set_adult_pawn(pawn* p)
	{
		m_almost_adult = p;
	}

	bool board::create_new_piece(uint8_t pti)
	{
		//try with getting the new piece by size() - 1
		if (!m_almost_adult) { return false; }
		switch ((piece_type_identifier)pti)
		{
		case piece_type_identifier::KNIGHT:
		{
			knight& ptrp = m_knights.emplace_back(knight());
			ptrp.init(this, m_almost_adult->get_color());
			ptrp.move(m_almost_adult->get_current_position(), true);
			ptrp.set_moved(false);
			m_almost_adult = nullptr;
			return true;
		}
		case piece_type_identifier::BISHOP:
		{
			bishop& ptrp = m_bishops.emplace_back(bishop());
			ptrp.init(this, m_almost_adult->get_color());
			ptrp.move(m_almost_adult->get_current_position(), true);
			ptrp.set_moved(false);
			m_almost_adult = nullptr;
			return true;
		}
		case piece_type_identifier::ROOK:
		{
			rook& ptrp = m_rooks.emplace_back(rook());
			ptrp.init(this, m_almost_adult->get_color());
			ptrp.move(m_almost_adult->get_current_position(), true);
			ptrp.set_moved(false);
			m_almost_adult = nullptr;
			return true;
		}
		case piece_type_identifier::QUEEN:
		{
			queen& ptrp = m_queens.emplace_back(queen());
			ptrp.init(this, m_almost_adult->get_color());
			ptrp.move(m_almost_adult->get_current_position(), true);
			ptrp.set_moved(false);
			m_almost_adult = nullptr;
			return true;
		}
		}

		Error(std::cout << '\n') << "Wrong number\n";
		return false;
	}

	void board::delete_last_piece(uint8_t pti)
	{
		switch ((piece_type_identifier)pti)
		{
			case piece_type_identifier::KNIGHT:
			{
				auto pos = m_knights[m_knights.size() - 1].get_current_position();
				m_block[(int)pos.x()][(int)pos.y()] = NULL;

				m_knights.resize(m_knights.size() - 1);
				return;
			}
			case piece_type_identifier::BISHOP:
			{
				auto pos = m_bishops[m_bishops.size() - 1].get_current_position();
				m_block[(int)pos.x()][(int)pos.y()] = NULL;

				m_bishops.resize(m_bishops.size() - 1);
				return;
			}
			case piece_type_identifier::ROOK:
			{
				auto pos = m_rooks[m_rooks.size() - 1].get_current_position();
				m_block[(int)pos.x()][(int)pos.y()] = NULL;

				m_rooks.resize(m_rooks.size() - 1);
				return;
			}
			case piece_type_identifier::QUEEN:
			{
				auto pos = m_queens[m_queens.size() - 1].get_current_position();
				m_block[(int)pos.x()][(int)pos.y()] = NULL;

				m_queens.resize(m_queens.size() - 1);
				return;
			}
		}
	}

	bool board::king_hitted(bool color) const
	{
		const king* our_king = nullptr;

		if (m_kings[0].get_color() == color)
			our_king = &m_kings[0];
		else
			our_king = &m_kings[1];

#if 1

#define AddPossimbleAttackerA(xin, yin) auto cn = m_block[ourKingPosition.x() + xin][ourKingPosition.y() + yin].get_class_number();\
		if (cn == UNKOWN)\
			continue;\
		if (m_block[ourKingPosition.x() + xin][ourKingPosition.y() + yin].get_object_color() == our_king->get_color()\
			|| cn == KNIGHT || cn == BISHOP || cn == PAWN)\
		{\
			break;\
		}\
		possimbleAttackers.push_back(vec2<float>(xin, yin));\
		break

#define AddPossimbleAttackerB(xin, yin) auto cn = m_block[ourKingPosition.x() + xin][ourKingPosition.y() + yin].get_class_number();\
		if (cn == UNKOWN)\
			continue;\
		if (m_block[ourKingPosition.x() + xin][ourKingPosition.y() + yin].get_object_color() == our_king->get_color()\
			|| cn == KNIGHT || cn == ROOK)\
			break;\
		possimbleAttackers.push_back(vec2<float>(xin, yin));\
		break

		//find all the blocks that can see our king
		vec2<float> ourKingPosition = our_king->get_current_position();
		std::vector<vec2<float>> possimbleAttackers;
		float right = 7 - ourKingPosition.x();
		float left = 0 - ourKingPosition.x();
		float up = 7 - ourKingPosition.y();
		float down = 0 - ourKingPosition.y();

		for (float y = 1; y < up + 1; y++)
		{
			AddPossimbleAttackerA(0, y);
		}
		for (float y = 1; y < mtcs::absolute(down) + 1; y++)
		{
			AddPossimbleAttackerA(0, -y);
		}
		for (float y = 1; y < right + 1; y++)
		{
			AddPossimbleAttackerA(y, 0);
		}
		for (float y = 1; y < mtcs::absolute(left) + 1; y++)
		{
			AddPossimbleAttackerA(-y, 0);
		}

		for (float y = 1; y < mtcs::get_smaller(up, right) + 1; y++)
		{
			AddPossimbleAttackerB(y, y);
		}
		for (float y = 1; y < mtcs::get_smaller(up, mtcs::absolute(left)) + 1; y++)
		{
			AddPossimbleAttackerB(-y, y);
		}
		for (float y = 1; y < mtcs::get_smaller(mtcs::absolute(down), mtcs::absolute(left)) + 1; y++)
		{
			AddPossimbleAttackerB(-y, -y);
		}
		for (float y = 1; y < mtcs::get_smaller(mtcs::absolute(down), right) + 1; y++)
		{
			AddPossimbleAttackerB(y, -y);
		}

		bool k_right = right >= 2;
		bool k_left = mtcs::absolute(left) >= 2;
		bool k_up = up >= 2;
		bool k_down = mtcs::absolute(down) >= 2;

		if (k_up)
		{
			if (mtcs::absolute(left) && m_block[ourKingPosition.x() - 1][ourKingPosition.y() + 2].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() - 1][ourKingPosition.y() + 2].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(-1, 2));
			if (right && m_block[ourKingPosition.x() + 1][ourKingPosition.y() + 2].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() + 1][ourKingPosition.y() + 2].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(1, 2));
		}
		if (k_down)
		{
			if (mtcs::absolute(left) && m_block[ourKingPosition.x() - 1][ourKingPosition.y() - 2].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() - 1][ourKingPosition.y() - 2].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(-1, -2));
			if (right && m_block[ourKingPosition.x() + 1][ourKingPosition.y() - 2].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() + 1][ourKingPosition.y() - 2].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(1, -2));
		}
		if (k_left)
		{
			if (mtcs::absolute(down) && m_block[ourKingPosition.x() - 2][ourKingPosition.y() - 1].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() - 2][ourKingPosition.y() - 1].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(-2, -1));
			if (up && m_block[ourKingPosition.x() - 2][ourKingPosition.y() + 1].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() - 2][ourKingPosition.y() + 1].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(-2, 1));
		}
		if (k_right)
		{
			if (mtcs::absolute(down) && m_block[ourKingPosition.x() + 2][ourKingPosition.y() - 1].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() + 2][ourKingPosition.y() - 1].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(2, -1));
			if (up && m_block[ourKingPosition.x() + 2][ourKingPosition.y() + 1].get_class_number() == KNIGHT
				&& m_block[ourKingPosition.x() + 2][ourKingPosition.y() + 1].get_object_color() != color)
				possimbleAttackers.push_back(vec2<float>(2, 1));
		}

#undef AddPossimbleAttackerA
#undef AddPossimbleAttackerB

		for (auto& posBlock : possimbleAttackers)
		{
			posBlock = posBlock + ourKingPosition;
			std::vector<vec2<float>>& current_obj_pm = m_block[posBlock.x()][posBlock.y()].contains<piece>()->possible_moves(false);
			for (int i = 0; i < current_obj_pm.size(); i++)
			{
				if (current_obj_pm[i] + m_block[posBlock.x()][posBlock.y()].contains<piece>()->get_current_position() == ourKingPosition)
					return true;
			}
		}
#else 
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				if (block[x][y].get_class_number() != UNKOWN && block[x][y].contains<piece>()->get_color() != color)
				{
					std::vector<vec2<float>>& current_obj_pm = block[x][y].contains<piece>()->possible_moves(false);
					for (int i = 0; i < current_obj_pm.size(); i++)
					{
						if (current_obj_pm[i] + block[x][y].contains<piece>()->get_current_position() == our_king->get_current_position())
							return true;
					}
				}
			}
		}
#endif
		return false;
	}

	void* board::kingSecurity(uint8_t pti)
	{
		return kingsecuritymap[pti].func;
	}
	
	bool board::check_for_ending(bool player)
	{
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				if (m_block[x][y].contains<void>() != nullptr && m_block[x][y].get_object_color() == player)
				{
					if (!m_block[x][y].contains<piece>()->possible_moves().empty())
						return false;
				}
			}
		}

		return true;
	}

	bool board::between_blocks(vec2<float> direction, vec2<float> position) const
	{
		vec2<float> one_block_move = (direction - position) / (vec2<float>::absolute(direction - position));
		
		vec2<float> z = position + one_block_move;
		while (9)
		{
			if (z == direction) { break; }
			if (m_block[(int)z.x()][(int)z.y()].contains<void>() != nullptr)
			{
				return false;
			}
			z = z + one_block_move;
		}

		return true;
	}

	void board::clear()
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				m_block[i][j] = NULL;
			}
		}

		m_pawns.clear();
		m_knights.clear();
		m_bishops.clear();
		m_rooks.clear();
		m_queens.clear();
		m_kings.clear();
	}

	void board::set_position(std::string& fen)
	{
		clear();

		m_pawns.reserve(16);
		m_knights.reserve(20);
		m_bishops.reserve(20);
		m_rooks.reserve(20);
		m_queens.reserve(18);
		m_kings.reserve(2);

		if (fen == "?" || fen == "") { fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; }

		//Reading FEN 

		std::string ColorTypePiecies;
		std::string Roke;
		std::string FiftyMoveRule = "";
		std::string Blackmove = "";

		int ylevel = 7, xlevel = 0;

		for (size_t i = 0; i < fen.size(); i++)
		{

			if (fen[i] == '/' || fen[i] == ' ') { ylevel -= 1; xlevel = 0; }
			else if (ylevel >= 0)
			{
				if (fen[i] > 'A')
				{
					player_color color;
					if (fen[i] > 'a') { ColorTypePiecies = "pnbrqk"; color = player_color::BLACK; }
					else { ColorTypePiecies = "PNBRQK"; color = player_color::WHITE; }

					switch (ColorTypePiecies.find(fen[i]) + 1)
					{
					case piece_type_identifier::PAWN:
					{
						m_pawns.emplace_back(pawn());
						m_pawns[m_pawns.size() - 1].init(this, color);
						m_pawns[m_pawns.size() - 1].move(vec2<float>((float)xlevel, (float)ylevel), true, false);
						m_pawns[m_pawns.size() - 1].set_moved(false);
					}
					break;
					case piece_type_identifier::KNIGHT:
					{
						m_knights.emplace_back(knight());
						m_knights[m_knights.size() - 1].init(this, color);
						m_knights[m_knights.size() - 1].move(vec2<float>((float)xlevel, (float)ylevel), true, false);
						m_knights[m_knights.size() - 1].set_moved(false);
					}
					break;
					case piece_type_identifier::BISHOP:
					{
						m_bishops.emplace_back(bishop());
						m_bishops[m_bishops.size() - 1].init(this, color);
						m_bishops[m_bishops.size() - 1].move(vec2<float>((float)xlevel, (float)ylevel), true, false);
						m_bishops[m_bishops.size() - 1].set_moved(false);
					}
					break;
					case piece_type_identifier::ROOK:
					{
						m_rooks.emplace_back(rook());
						m_rooks[m_rooks.size() - 1].init(this, color);
						m_rooks[m_rooks.size() - 1].move(vec2<float>((float)xlevel, (float)ylevel), true, false);
						m_rooks[m_rooks.size() - 1].set_moved(false);
					}
					break;
					case piece_type_identifier::QUEEN:
					{
						m_queens.emplace_back(queen());
						m_queens[m_queens.size() - 1].init(this, color);
						m_queens[m_queens.size() - 1].move(vec2<float>((float)xlevel, (float)ylevel), true, false);
						m_queens[m_queens.size() - 1].set_moved(false);
					}
					break;
					case piece_type_identifier::KING:
					{
						m_kings.emplace_back(king());
						m_kings[m_kings.size() - 1].init(this, color);
						m_kings[m_kings.size() - 1].move(vec2<float>((float)xlevel, (float)ylevel), true, false);
						m_kings[m_kings.size() - 1].set_moved(false);
					}
					break;
					}

					xlevel += 1;
				}
				else
				{
					xlevel += (int)fen[i] - 48;
				}
			}
			else if (ylevel == -1)
			{
				if (fen[i] == 'w') { m_player = true; }
				else { m_player = false; }
			}
			else if (ylevel == -2)
			{
				if (fen[i] == '-')
				{
					for (auto& r : m_rooks)
						r.set_moved(true);
				}
				else
				{
					Roke += fen[i];
				}
			}
			else if (ylevel == -3)
			{
				if (fen[i] == '-') { ylevel -= 1; continue; }

				if (fen[i] >= 'a')
					m_lastmovedir.x() = (float)(int)(fen[i] - 'a');
				else if (fen[i] == '6')
					m_lastmovedir.y() = 4;
				else
					m_lastmovedir.y() = 3;
			}
			else if (ylevel == -4)
			{
				FiftyMoveRule += fen[i];
			}
			else
			{
				Blackmove += fen[i];
			}

		}

		if (!(Roke.find('K') + 1))
		{
			if (m_block[7][0].get_class_number() == piece_type_identifier::ROOK)
				m_block[7][0].contains<rook>()->set_moved(true);
		}
		if (!(Roke.find('Q') + 1))
		{
			if (m_block[0][0].get_class_number() == piece_type_identifier::ROOK)
				m_block[0][0].contains<rook>()->set_moved(true);
		}
		if (!(Roke.find('k') + 1))
		{
			if (m_block[7][7].get_class_number() == piece_type_identifier::ROOK)
				m_block[7][7].contains<rook>()->set_moved(true);
		}
		if (!(Roke.find('q') + 1))
		{
			if (m_block[0][7].get_class_number() == piece_type_identifier::ROOK)
				m_block[0][7].contains<rook>()->set_moved(true);
		}

		for (int i = 0; i < m_kings.size(); i++)
		{
			if (m_kings[i].get_current_position() != vec2<float>(4, -7 * (m_kings[i].get_color() - 1)))
				m_kings[i].set_moved(true);
		}

		m_FiftyMoveRule = mtcs::trans_int(FiftyMoveRule);
		m_BlackMoves = mtcs::trans_int(Blackmove);

		PositionERROR();
	}
}
