#include "chess_pecie.h"

#include "chess_board.h"
#include "chess_blocks.h"

#include "math_Lib.h"


namespace chess_core
{
	//piece

	piece::piece()
	{
		m_current_position = vec2<float>(0, 0);
		m_game = nullptr;
		m_moved = false;
		m_color = NULL;
	}

	void piece::init(board* game, bool color)
	{
		m_color = color;
		m_game = game;
	}

	void piece::set_color(bool color)
	{
		m_color = color;
	}

	void piece::set_moved(bool moved)
	{
		m_moved = moved;
	}

	void piece::set_board(board* game)
	{
		m_game = game;
	}

	vec2<float>& piece::get_current_position()
	{
		return m_current_position;
	}

	vec2<float> piece::get_current_position() const
	{
		return m_current_position;
	}

	bool piece::get_color() const
	{
		return m_color;
	}

	bool piece::get_moved() const
	{
		return m_moved;
	}

	//-----------


	//pawn

	pawn::pawn()
		:piece()
	{

	}

	std::vector<vec2<float>>& pawn::possible_moves(bool real) const
	{
		m_possibleMoves.clear();
		if (m_current_position.y() == 0 || m_current_position.y() == 7) { return m_possibleMoves; }
		if (m_color)
		{
			m_possibleMoves.push_back(vec2<float>(0, 1));
			if (m_current_position.x() != 7) { m_possibleMoves.push_back(vec2<float>(1, 1)); }
			if (m_current_position.x() != 0) { m_possibleMoves.push_back(vec2<float>(-1, 1)); }
			if (m_current_position.y() == 1) { m_possibleMoves.push_back(vec2<float>(0, 2)); }
		}
		else
		{
			m_possibleMoves.push_back(vec2<float>(0, -1));
			if (m_current_position.x() != 7) { m_possibleMoves.push_back(vec2<float>(1, -1)); }
			if (m_current_position.x() != 0) { m_possibleMoves.push_back(vec2<float>(-1, -1)); }
			if (m_current_position.y() == 6) { m_possibleMoves.push_back(vec2<float>(0, -2)); }
		}

		vec2<float> direction;
		for (int z = 0; z < m_possibleMoves.size(); z++)
		{
			direction = m_possibleMoves[z] + m_current_position;

			//double move
			if (mtcs::absolute(m_possibleMoves[z].y()) == 2.0f)
			{
				if (m_game->get_blocks(direction).contains<void>() == nullptr
					&& m_game->get_blocks(m_current_position.x(), m_current_position.y() + m_possibleMoves[z].y() / 2).contains<void>() == nullptr
					&& (m_current_position.y() == 1.0f
						|| m_current_position.y() == 6.0f))
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());				
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<pawn>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}
			}

			//move one
			else if (m_game->get_blocks(direction).contains<void>() == nullptr
				&& mtcs::absolute(m_possibleMoves[z].x()) != mtcs::absolute(m_possibleMoves[z].y()))
			{
				auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
				if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<pawn>(), direction, real))
				{
					continue;
				}
				m_possibleMoves.erase(m_possibleMoves.begin() + z);
				z -= 1;
				continue;
			}

			//copy
			if (m_game->get_blocks(direction).contains<void>() != nullptr
				&& mtcs::absolute(m_possibleMoves[z].x()) == mtcs::absolute(m_possibleMoves[z].y())
				&& m_game->get_blocks(direction).get_object_color() != m_color)
			{
				auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
				if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<pawn>(), direction, real))
				{
					continue;
				}
				m_possibleMoves.erase(m_possibleMoves.begin() + z);
				z -= 1;
				continue;
			}

			//an pan san
			if (m_game->get_blocks(direction).contains<void>() == nullptr
				&& m_game->get_blocks((int)direction.x(), (int)m_current_position.y()).get_class_number() == piece_type_identifier::PAWN
				&& m_game->get_blocks((int)direction.x(), (int)m_current_position.y()).get_object_color() != m_color)
			{
				if (m_game->get_blocks((int)direction.x(), (int)m_current_position.y()).contains<pawn>()->get_current_position() == m_game->get_last_move_dir())
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<pawn>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}
			}
			m_possibleMoves.erase(m_possibleMoves.begin() + z);
			z -= 1;
			continue;
		}

		return m_possibleMoves;
	}

	bool pawn::move(vec2<float> direction, bool first_time/* = false*/, bool real/* = true*/)
	{
		if (real && !first_time)
		{
			if (mtcs::absolute(direction.y() - m_current_position.y()) == 2.0)
				m_game->set_last_move_dir(direction);
			else
				m_game->set_last_move_dir(vec2<float>(0, 0));

			if (m_game->Is_pawn_waiting())
				return false;

			if (m_color == 0)
				m_game->get_black_moves() += 1;
			m_game->get_fifty_move_rule() = 0;
		}

		if (!first_time && m_game->get_blocks(direction).contains<void>() == nullptr
			&& (bool)mtcs::get_bigger(direction.y() - m_current_position.y(), 0.0f) == m_color
			&& mtcs::absolute(direction.x() - m_current_position.x()) == mtcs::absolute(direction.y() - m_current_position.y()))
		{
			m_game->get_blocks((int)direction.x(), (int)m_current_position.y()) = NULL;
			if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }
			m_current_position = direction;
			m_game->get_blocks(m_current_position) = this;
			m_moved = true;
		}
		else
		{
			if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }
			m_current_position = direction;
			m_game->get_blocks(m_current_position) = this;
			m_moved = true;
			if (real && (m_current_position.y() == 0.0f || m_current_position.y() == 7.0f))
			{
				m_game->set_adult_pawn(this);
			}
		}
		return true;
	}

	//bishop

	bishop::bishop()
		:piece()
	{

	}

	std::vector<vec2<float>>& bishop::possible_moves(bool real) const
	{
		
		m_possibleMoves.clear();
		float right = 7 - m_current_position.x();
		float left = 0 - m_current_position.x();
		float up = 7 - m_current_position.y();
		float down = 0 - m_current_position.y();

		for (float y = 1; y < mtcs::get_smaller(up, right) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(y, y));
		}
		for (float y = 1; y < mtcs::get_smaller(up, mtcs::absolute(left)) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(-y, y));
		}
		for (float y = 1; y < mtcs::get_smaller(mtcs::absolute(down), mtcs::absolute(left)) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(-y, -y));
		}
		for (float y = 1; y < mtcs::get_smaller(mtcs::absolute(down), right) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(y, -y));
		}

		
		vec2<float> direction;
		for (int z = 0; z < m_possibleMoves.size(); z++)
		{
			direction = m_possibleMoves[z] + m_current_position;

			//no other piecies between the position and direction
			if (m_game->between_blocks(direction, m_current_position))
			{
				//regular move
				if (m_game->get_blocks(direction).contains<void>() == nullptr)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<bishop>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

				//copy
				if (m_game->get_blocks(direction).contains<void>() != nullptr
					&& m_game->get_blocks(direction).get_object_color() != m_color)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<bishop>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}
			}
			m_possibleMoves.erase(m_possibleMoves.begin() + z);
			z -= 1;
			continue;
		}

		return m_possibleMoves;
	}

	bool bishop::move(vec2<float> direction, bool first_time/* = false*/, bool real/* = true*/)
	{
		if (real && !first_time)
		{
			m_game->set_last_move_dir(vec2<float>(0, 0));
			if (m_game->Is_pawn_waiting())
				return false;
			if (m_color == 0)
				m_game->get_black_moves() += 1;
			if (m_game->get_blocks(direction).get_class_number() != UNKOWN)
				m_game->get_fifty_move_rule() = 0;
		}

		if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }
		m_current_position = direction;
		m_game->get_blocks(m_current_position) = this;
		m_moved = true;
		return true;
	}

	//rook

	rook::rook()
		:piece()
	{

	}

	std::vector<vec2<float>>& rook::possible_moves(bool real) const
	{
		
		m_possibleMoves.clear();
		float right = 7 - m_current_position.x();
		float left = 0 - m_current_position.x();
		float up = 7 - m_current_position.y();
		float down = 0 - m_current_position.y();

		for (float y = 1; y < up + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(0, y));
		}
		for (float y = 1; y < mtcs::absolute(down) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(0, -y));
		}
		for (float y = 1; y < right + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(y, 0));
		}
		for (float y = 1; y < mtcs::absolute(left) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(-y, 0));
		}

		
		vec2<float> direction;
		for (int z = 0; z < m_possibleMoves.size(); z++)
		{
			direction = m_possibleMoves[z] + m_current_position;


			//no other piecies between the position and direction
			if (m_game->between_blocks(direction, m_current_position))
			{

				//regular move
				if (m_game->get_blocks(direction).contains<void>() == nullptr)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<rook>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

				//copy
				if (m_game->get_blocks(direction).contains<void>() != nullptr
					&& m_game->get_blocks(direction).get_object_color() != m_color)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<rook>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

			}
			m_possibleMoves.erase(m_possibleMoves.begin() + z);
			z -= 1;
			continue;
		}

		return m_possibleMoves;
	}

	bool rook::move(vec2<float> direction, bool first_time/* = false*/, bool real/* = true*/)
	{
		if (real && !first_time)
		{
			m_game->set_last_move_dir(vec2<float>(0, 0));
			if (m_game->Is_pawn_waiting())
				return false;
			if (m_color == 0)
				m_game->get_black_moves() += 1;
			if (m_game->get_blocks(direction).get_class_number() != UNKOWN)
				m_game->get_fifty_move_rule() = 0;
		}

		if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }
		m_current_position = direction;
		m_game->get_blocks(m_current_position) = this;
		m_moved = true;
		return true;
	}

	//queen

	queen::queen()
		:piece()
	{

	}

	std::vector<vec2<float>>& queen::possible_moves(bool real) const
	{
		
		m_possibleMoves.clear();
		float right = 7 - m_current_position.x();
		float left = 0 - m_current_position.x();
		float up = 7 - m_current_position.y();
		float down = 0 - m_current_position.y();

		for (float y = 1; y < up + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(0, y));
		}
		for (float y = 1; y < mtcs::absolute(down) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(0, -y));
		}
		for (float y = 1; y < right + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(y, 0));
		}
		for (float y = 1; y < mtcs::absolute(left) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(-y, 0));
		}
		for (float y = 1; y < mtcs::get_smaller(up, right) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(y, y));
		}
		for (float y = 1; y < mtcs::get_smaller(up, mtcs::absolute(left)) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(-y, y));
		}
		for (float y = 1; y < mtcs::get_smaller(mtcs::absolute(down), mtcs::absolute(left)) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(-y, -y));
		}
		for (float y = 1; y < mtcs::get_smaller(mtcs::absolute(down), right) + 1; y++)
		{
			m_possibleMoves.push_back(vec2<float>(y, -y));
		}

		
		vec2<float> direction;
		for (int z = 0; z < m_possibleMoves.size(); z++)
		{
			int cr = 0;

			direction = m_possibleMoves[z] + m_current_position;

			//no other piecies between the position and direction
			if (m_game->between_blocks(direction, m_current_position))
			{

				//regular move
				if (m_game->get_blocks(direction).contains<void>() == nullptr)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<queen>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

				//copy
				if (m_game->get_blocks(direction).contains<void>() != nullptr &&
					m_game->get_blocks(direction).get_object_color() != m_color)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<queen>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

			}
			m_possibleMoves.erase(m_possibleMoves.begin() + z);
			z -= 1;
			continue;
		}

		return m_possibleMoves;
	}

	bool queen::move(vec2<float> direction, bool first_time/* = false*/, bool real/* = true*/)
	{
		if (real && !first_time)
		{
			m_game->set_last_move_dir(vec2<float>(0, 0));
			if (m_game->Is_pawn_waiting())
				return false;
			if (m_color == 0)
				m_game->get_black_moves() += 1;
			if (m_game->get_blocks(direction).get_class_number() != UNKOWN)
				m_game->get_fifty_move_rule() = 0;
		}

		if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }
		m_current_position = direction;
		m_game->get_blocks(m_current_position) = this;
		m_moved = true;
		return true;
	}

	//king

	king::king()
		:piece()
	{

	}

	std::vector<vec2<float>>& king::possible_moves(bool real) const
	{
		
		m_possibleMoves.clear();
		float right = 7 - m_current_position.x();
		float left = 0 - m_current_position.x();
		float up = 7 - m_current_position.y();
		float down = 0 - m_current_position.y();

		bool k_right = right >= 1;
		bool k_left = mtcs::absolute(left) >= 1;
		bool k_up = up >= 1;
		bool k_down = mtcs::absolute(down) >= 1;

		if (k_up)
		{
			m_possibleMoves.push_back(vec2<float>(0, 1));
		}
		if (k_down)
		{
			m_possibleMoves.push_back(vec2<float>(0, -1));
		}
		if (k_left)
		{
			m_possibleMoves.push_back(vec2<float>(-1, 0));
		}
		if (k_right)
		{
			m_possibleMoves.push_back(vec2<float>(1, 0));
		}
		if (k_up && k_left)
		{
			m_possibleMoves.push_back(vec2<float>(-1, 1));
		}
		if (k_up && k_right)
		{
			m_possibleMoves.push_back(vec2<float>(1, 1));
		}
		if (k_down && k_left)
		{
			m_possibleMoves.push_back(vec2<float>(-1, -1));
		}
		if (k_down && k_right)
		{
			m_possibleMoves.push_back(vec2<float>(1, -1));
		}
		if (!m_moved)
		{
			m_possibleMoves.push_back(vec2<float>(2, 0));
			m_possibleMoves.push_back(vec2<float>(-2, 0));
		}

		
		vec2<float> direction;
		for (int z = 0; z < m_possibleMoves.size(); z++)
		{
			int cr = 0;

			direction = m_possibleMoves[z] + m_current_position;

			//roke
			if (mtcs::absolute(m_possibleMoves[z].x()) == 2)
			{
				if (m_possibleMoves[z].x() < 0.0f)
				{
					//queen roke
					vec2<float> rook_position = direction + vec2<float>(-2, 0);

					//no other piecies between the position and direction
					if (m_game->between_blocks(rook_position, m_current_position))
					{
						cr += 1;
					}

					if (m_game->get_blocks(rook_position).get_class_number() == piece_type_identifier::ROOK)
					{
						//the rook is available for roke
						if (!m_game->get_blocks(rook_position).contains<rook>()->get_moved())
						{
							cr += 1;
						}
					}
				}
				else
				{
					//king roke
					vec2<float> rook_position = direction + vec2<float>(1, 0);

					//no other piecies between the position and direction
					if (m_game->between_blocks(rook_position, m_current_position))
					{
						cr += 1;
					}

					if (m_game->get_blocks(rook_position).get_class_number() == piece_type_identifier::ROOK)
					{
						//the rook is available for roke
						if (!m_game->get_blocks(rook_position).contains<rook>()->get_moved())
						{
							cr += 1;
						}
					}
				}

				if (cr >= 2)
				{
					//check the three positions tha the king has to pass, to see if he is secured
					auto kingSecurityFunc1 = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					auto kingSecurityFunc2 = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(m_current_position + m_possibleMoves[z] / vec2<float>(2, 1)).get_class_number());
					auto kingSecurityFunc3 = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(m_current_position).get_class_number());
					if (kingSecurityFunc1(m_game, m_game->get_blocks(m_current_position).contains<king>(), direction, real) &&
						kingSecurityFunc2(m_game, m_game->get_blocks(m_current_position).contains<king>(), m_current_position + m_possibleMoves[z] / vec2<float>(2, 1), real) &&
						kingSecurityFunc3(m_game, m_game->get_blocks(m_current_position).contains<king>(), m_current_position, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

			}//end-roke
			else
			{
				//regular move
				if (m_game->get_blocks(direction).contains<void>() == nullptr)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<king>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}

				//copy
				if (m_game->get_blocks(direction).contains<void>() != nullptr
					&& m_game->get_blocks(direction).get_object_color() != m_color)
				{
					auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
					if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<king>(), direction, real))
					{
						continue;
					}
					m_possibleMoves.erase(m_possibleMoves.begin() + z);
					z -= 1;
					continue;
				}
			}
			m_possibleMoves.erase(m_possibleMoves.begin() + z);
			z -= 1;
			continue;
		}

		return m_possibleMoves;
	}

	bool king::move(vec2<float> direction, bool first_time/* = false*/, bool real/* = true*/)
	{
		if (real && !first_time)
		{
			m_game->set_last_move_dir(vec2<float>(0, 0));
			if (m_game->Is_pawn_waiting())
				return false;
			if (m_color == 0)
				m_game->get_black_moves() += 1;
			if (m_game->get_blocks(direction).get_class_number() != UNKOWN)
				m_game->get_fifty_move_rule() = 0;
		}

		bool roke = false;

		if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }

		if (mtcs::absolute(direction.x() - m_current_position.x()) == 2 && !first_time)
		{
			if (!m_moved)
				roke = true;
		}

		m_current_position = direction;
		m_game->get_blocks(m_current_position) = this;
		m_moved = true;

		if (roke)
		{
			vec2<float> rook_position = vec2<float>((float)mtcs::get_closer(0, 7, (int)m_current_position.x()), m_current_position.y());
			vec2<float> rook_direction = vec2<float>((float)mtcs::get_closer(3, 5, (int)m_current_position.x()), m_current_position.y());

			m_game->get_blocks(rook_position).contains<rook>()->move(rook_direction, false, false);
		}
		return true;
	}

	//knight

	knight::knight()
		:piece()
	{

	}

	std::vector<vec2<float>>& knight::possible_moves(bool real) const
	{
		
		m_possibleMoves.clear();
		float right = 7 - m_current_position.x();
		float left = 0 - m_current_position.x();
		float up = 7 - m_current_position.y();
		float down = 0 - m_current_position.y();

		bool k_right = right >= 2;
		bool k_left = mtcs::absolute(left) >= 2;
		bool k_up = up >= 2;
		bool k_down = mtcs::absolute(down) >= 2;

		if (k_up)
		{
			if (mtcs::absolute(left))
				m_possibleMoves.push_back(vec2<float>(-1, 2));
			if (right)
				m_possibleMoves.push_back(vec2<float>(1, 2));
		}
		if (k_down)
		{
			if (mtcs::absolute(left))
				m_possibleMoves.push_back(vec2<float>(-1, -2));
			if (right)
				m_possibleMoves.push_back(vec2<float>(1, -2));
		}
		if (k_left)
		{
			if (mtcs::absolute(down))
				m_possibleMoves.push_back(vec2<float>(-2, -1));
			if (up)
				m_possibleMoves.push_back(vec2<float>(-2, 1));
		}
		if (k_right)
		{
			if (mtcs::absolute(down))
				m_possibleMoves.push_back(vec2<float>(2, -1));
			if (up)
				m_possibleMoves.push_back(vec2<float>(2, 1));
		}

		
		vec2<float> direction;
		for (int z = 0; z < m_possibleMoves.size(); z++)
		{
			int cr = 0;

			direction = m_possibleMoves[z] + m_current_position;
			
			//regular move
			if (m_game->get_blocks(direction).contains<void>() == nullptr)
			{
				auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
				if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<knight>(), direction, real))
				{
					continue;
				}
				m_possibleMoves.erase(m_possibleMoves.begin() + z);
				z -= 1;
				continue;
			}

			//copy
			if (m_game->get_blocks(direction).contains<void>() != nullptr
				&& m_game->get_blocks(direction).get_object_color() != m_color)
			{
				auto kingSecurityFunc = (bool(*)(board*, piece*, vec2<float>, bool))m_game->kingSecurity(m_game->get_blocks(direction).get_class_number());
				if (kingSecurityFunc(m_game, m_game->get_blocks(m_current_position).contains<knight>(), direction, real))
				{
					continue;
				}
				m_possibleMoves.erase(m_possibleMoves.begin() + z);
				z -= 1;
				continue;
			}
			m_possibleMoves.erase(m_possibleMoves.begin() + z);
			z -= 1;
			continue;
		}

		return m_possibleMoves;
	}

	bool knight::move(vec2<float> direction, bool first_time/* = false*/, bool real/* = true*/)
	{
		if (real && !first_time)
		{
			m_game->set_last_move_dir(vec2<float>(0, 0));
			if (m_game->Is_pawn_waiting())
				return false;
			if (m_color == 0)
				m_game->get_black_moves() += 1;
			if (m_game->get_blocks(direction).get_class_number() != UNKOWN)
				m_game->get_fifty_move_rule() = 0;
		}

		if (!first_time) { m_game->get_blocks(m_current_position) = NULL; }
		m_current_position = direction;
		m_game->get_blocks(m_current_position) = this;
		m_moved = true;
		return true;
	}
}
