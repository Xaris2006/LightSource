#pragma once

#include <iostream>

namespace Chess
{
	class BitBoard
	{
	public:
		BitBoard() = default;
		~BitBoard() = default;

		bool At(int index) const;
		bool At(int indexX, int indexY) const;
		void Set(int index, bool value);
		void Set(int indexX, int indexY, bool value);

		uint64_t& Data();
		uint64_t Data() const;

		BitBoard operator+(const BitBoard& other) const;

	private:
		uint64_t m_data = 0;

	};
}
