#pragma once

#include "vec2/vec2_lib.h"

#include <vector>


namespace chess_core
{
	class board;
	
	enum player_color : uint8_t
	{
		BLACK,
		WHITE
	};

	enum piece_type_identifier : uint8_t
	{
		UNKOWN,
		PAWN, KNIGHT, BISHOP,
		ROOK, QUEEN, KING
	};

	class piece
	{
	public:
		piece();
		~piece() = default;

		void init(board* game, bool color);
		void set_color(bool color);
		void set_moved(bool moved);
		void set_board(board* game);

		vec2<float>& get_current_position();
		vec2<float> get_current_position() const;
		bool get_color() const;
		bool get_moved() const;
		
		//function: possible_moves  -> find generally the possible moves of the object
		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const = 0;
		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) = 0;

	protected:
		board* m_game;

		bool m_color;
		bool m_moved;
		vec2<float> m_current_position;
		mutable std::vector<vec2<float>> m_possibleMoves;
	};

	//pawn
	class pawn : public piece
	{
	public:
		pawn();
		~pawn() = default;

		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const override;

		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) override;
	};

	//bishop
	class bishop : public piece
	{
	public:
		bishop();
		~bishop() = default;

		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const override;

		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) override;
	};

	//knight
	class knight : public piece
	{
	public:
		knight();
		~knight() = default;

		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const override;

		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) override;
	};

	//rook
	class rook : public piece
	{
	public:
		rook();
		~rook() = default;

		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const override;

		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) override;

	};

	//queen
	class queen : public piece
	{
	public:
		queen();
		~queen() = default;

		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const override;

		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) override;

	};

	//king
	class king : public piece
	{
	public:
		king();
		~king() = default;

		virtual std::vector<vec2<float>>& possible_moves(bool real = true) const override;

		virtual bool move(vec2<float> direction, bool first_time = false, bool real = true) override;

	};

}