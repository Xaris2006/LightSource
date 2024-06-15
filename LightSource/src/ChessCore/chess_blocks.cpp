#include "chess_blocks.h"

#include "chess_board.h"


namespace chess_core
{
	//blocks

	blocks::blocks()
	{
		m_object = nullptr;

		m_class_number = piece_type_identifier::UNKOWN;
		m_object_color = player_color::BLACK;
	}

	piece_type_identifier blocks::get_class_number() const
	{
		return m_class_number;
	}

	bool blocks::get_object_color() const
	{
		return m_object_color;
	}

	bool blocks::get_moved() const
	{
		if (m_class_number)
			return m_object->get_moved();
		return false;
	}

	piece* blocks::get_piece_ptr(piece_type_identifier& cn) const
	{
		cn = m_class_number;
		if (m_class_number)
			return m_object;
		return nullptr;
	}

	void blocks::set_contained_piece(piece* object, piece_type_identifier pti)
	{
		m_class_number = pti;
		m_object_color = object->get_color();

		m_object = object;
	}

	void blocks::operator=(pawn* o)
	{
		m_class_number = piece_type_identifier::PAWN;
		m_object_color = o->get_color();

		m_object = o;
	}
	void blocks::operator=(knight* o)
	{
		m_class_number = piece_type_identifier::KNIGHT;
		m_object_color = o->get_color();

		m_object = o;
	}
	void blocks::operator=(bishop* o)
	{
		m_class_number = piece_type_identifier::BISHOP;
		m_object_color = o->get_color();

		m_object = o;
	}
	void blocks::operator=(rook* o)
	{
		m_class_number = piece_type_identifier::ROOK;
		m_object_color = o->get_color();

		m_object = o;
	}
	void blocks::operator=(queen* o)
	{
		m_class_number = piece_type_identifier::QUEEN;
		m_object_color = o->get_color();

		m_object = o;
	}
	void blocks::operator=(king* o)
	{
		m_class_number = piece_type_identifier::KING;
		m_object_color = o->get_color();

		m_object = o;
	}
	void blocks::operator=(int&& o)
	{
		m_class_number = piece_type_identifier::UNKOWN;
		m_object = nullptr;
	}


}