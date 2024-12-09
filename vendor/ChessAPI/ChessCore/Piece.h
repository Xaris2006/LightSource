#pragma once

#include <iostream>

namespace Chess
{
	enum Piece : uint8_t
	{
		PAWN	 = 0x00,
		KNIGHT	 = 0x01,
		BISHOP	 = 0x02,
		ROOK	 = 0x03,
		QUEEN	 = 0x04,
		KING	 = 0x05,
		NONE	 = 0x06,
		PROMOTED = 0x07
	};

	enum Color : bool
	{
		BLACK = 0,
		WHITE = 1
	};
	
	using MoveDifference = int8_t;

	constexpr MoveDifference PawnMoves  [] = { 8, 16, 9, 7 };
	constexpr MoveDifference KnightMoves[] = { 6, 10, 17, 15 };
	constexpr MoveDifference BishopMoves[] = { 7, 14, 21, 28, 35, 42, 49, 56, 9, 18, 27, 36, 45, 54, 63 };
	constexpr MoveDifference RookMoves  [] = { 1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56 };
	constexpr MoveDifference QueenMoves [] = { 7, 14, 21, 28, 35, 42, 49, 56, 9, 18, 27, 36, 45, 54, 63,
									1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56 };
	constexpr MoveDifference KingMoves  [] = { 1, 7, 8, 9 };
}