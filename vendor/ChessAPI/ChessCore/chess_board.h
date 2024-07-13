#pragma once

#include "vec2/vec2_lib.h"

#include "chess_blocks.h"

#include <vector>
#include <array>

namespace chess_core
{
	class piece;
	class pawn;
	class knight;
	class bishop;
	class rook;
	class queen;
	class king;

	class board
	{
	public:
		
		board();
		~board();

		//function: king_security -> after moving the object to the direction, 
		//we check if our king is being hitted
		void* kingSecurity(uint8_t pti);
		bool king_hitted(bool color) const;

		bool between_blocks(vec2<float> direction, vec2<float> position) const;

		bool check_for_ending(bool player);

		blocks& get_blocks(vec2<float> position);

		blocks& get_blocks(int x, int y);

		vec2<float> get_last_move_dir() const;

		bool& get_player_color();

		std::vector<piece*> get_ptr_piece(uint8_t cn) const;

		std::string get_FEN() const;

		std::vector<uint8_t> get_formated_position() const;

		unsigned int& get_fifty_move_rule();
		unsigned int& get_black_moves();
		void set_last_move_dir(vec2<float> last_move_dir);

		bool Is_pawn_waiting() const;
		void stop_waiting();
		void set_adult_pawn(pawn* p);
		//pti should be knight - bishop - rook - queen
		bool create_new_piece(uint8_t pti);
		void delete_last_piece(uint8_t pti);

		void clear();

		void set_position(std::string& fen);

		bool ERROR_EXIT;
		
	private:
		void PositionERROR();

		std::array<std::array<blocks, 8>, 8> m_block;
		std::vector<pawn>	m_pawns;
		std::vector<knight> m_knights;
		std::vector<bishop> m_bishops;
		std::vector<rook>	m_rooks;
		std::vector<queen>	m_queens;
		std::vector<king>	m_kings;

		pawn* m_almost_adult = nullptr;

		bool m_player = 1;
		vec2<float> m_lastmovedir = vec2<float>(0, 0);
		unsigned int m_FiftyMoveRule = 0;
		unsigned int m_BlackMoves = 1;
	};

}
