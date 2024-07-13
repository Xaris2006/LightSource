#include "math_Lib.h"

//#include <cmath>

#include "macros.h"

namespace mtcs
{

	int I(int number)
	{
		int output = 1;
		if (number == 0) { return output; }
		while (number != 1)
		{
			output *= number;
			number -= 1;
			if (number == 0) { return output; }
		}
		return output;
	}

	int ana(int size, int amount)
	{
		return I(size) / (I(amount) * I(size - amount));
	}

	int para(int size, int amount)
	{
		return ana(size + amount - 1, amount);
	}

	int find_max_diff_potisions(int size)
	{
		int output = 0;
		if (size - 1 == 0) { return output; }
		int m_size = size;
		while (true)
		{
			output += I(m_size) * ana(size, m_size);
			m_size -= 1;
			if (m_size == 0) { return output; }
		}
	}

	unsigned int trans_int(std::string number)
	{
		int output = 0;
		for (int y = (int)number.size() - 1; y > -1; y--)
		{
			output += ((int)number[y] - 48) * (int)pow(10, (number.size() - y - 1));
		}
		return output;
	}

	unsigned int trans_int(char number)
	{
		return ((int)number - 48);
	}

	std::string trans_str(int number)
	{
		std::string output = "";
		int max_level;
		int level = 0;

		while (true)
		{
			level += 1;
			if ((float)number / (float)std::pow(10, level) < 1.0f) { max_level = level; break; }
		}

		int extrnum = 0;

		for (int i = max_level - 1; i > -1; i--)
		{
			output = output + char(int(number / std::pow(10, i)) - extrnum + 48);
			extrnum = int(number / std::pow(10, i));
			extrnum *= 10;
		}

		return output;
	}
}