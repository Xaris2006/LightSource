#pragma once

#include <vector>
#include "Walnut/Core/Buffer.h"

class ArenaBuffer
{
public:
	static void Allocate(uint64_t size);
	static void Deallocate();

public:
	ArenaBuffer(uint64_t size);

	uint8_t& operator[](int index)
	{
		return s_buffer[index + m_offset];
	}

	uint8_t operator[](int index) const
	{
		return s_buffer[index + m_offset];
	}

	template<typename T>
	T* As() const
	{
		return (T*)s_buffer[m_offset];
	}

	inline uint64_t GetSize() const { return m_size; }

private:
	static Walnut::Buffer s_buffer;
	static std::vector<size_t> s_offsets;
	static std::vector<size_t> s_sizes;

private:
	size_t m_offset;
	size_t m_size;
};