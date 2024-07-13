#pragma once

#include "chess_pecie.h"

namespace chess_core
{
	class blocks
	{
	public:
		blocks();
		~blocks() = default;

		bool get_object_color() const;
		bool get_moved() const;
		piece* get_piece_ptr(piece_type_identifier& cn) const;
		piece_type_identifier get_class_number() const;

		//function: contains -> returns three possible solutions:
		//1: a pointer of the object that has the same coordinates of the block
		//2: a nullptr if there is no object with the same coordinates of the block
		//3: a pointer of a variable to just show that the block has the same coordinates of one of the objects

		template<typename I>
		I* contains() const
		{
			return (I*)m_object;
		}

		void set_contained_piece(piece* object, piece_type_identifier pti);
		void operator=(pawn* o);
		void operator=(knight* o);
		void operator=(bishop* o);
		void operator=(rook* o);
		void operator=(queen* o);
		void operator=(king* o);
		void operator=(int&& o);
	private:

		//variable: objec_color -> it helps us to return the right pointer as we know:
		//1 = pawn
		//2 = knight
		//3 = bishop
		//4 = rook
		//5 = queen
		//6 = king
		//0 = nullptr
		bool m_object_color;
		piece_type_identifier m_class_number;

		piece* m_object;
	};
}