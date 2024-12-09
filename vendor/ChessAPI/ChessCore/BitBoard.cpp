#include "BitBoard.h"

namespace Chess
{
	bool BitBoard::At(int index) const
	{
		return (bool)(1ULL << index & m_data);
	}
	
	bool BitBoard::At(int indexX, int indexY) const
	{
		return (bool)(1ULL << (indexX + 8 * indexY) & m_data);
	}

	void BitBoard::Set(int index, bool value)
	{
		if (value)
			m_data |= (1ULL << index);
		else
			m_data &= ~(1ULL << index);
	}

	void BitBoard::Set(int indexX, int indexY, bool value)
	{
		if (value)
			m_data |= (1ULL << (indexX + 8 * indexY));
		else
			m_data &= ~(1ULL << (indexX + 8 * indexY));
	}

	uint64_t& BitBoard::Data()
	{
		return m_data;
	}

	uint64_t BitBoard::Data() const
	{
		return m_data;
	}

	BitBoard BitBoard::operator+(const BitBoard& other) const
	{
		BitBoard ret;
		ret.m_data = m_data | other.m_data;
		return ret;
	}
}