#pragma once


#include <iostream>
#include <vector>


namespace mtcs
{

	int I(int number);

	int ana(int size, int amount);
	
	int para(int size, int amount);

	unsigned int trans_int(std::string number);

	unsigned int trans_int(char number);

	std::string trans_str(int number);

	int find_max_diff_potisions(int size);

	template<typename T>
	T absolute(T number)
	{
		if (number >= 0) { return number; }
		else { return -number; }
	}

	template<typename T>
	T find_persentage(T max, T correct)
	{
		return correct * 100 / max;
	}

	template<typename T>
	T get_smaller(T first, T second)
	{
		if (first <= second)
		{
			return first;
		}
		else
			return second;
	}

	template<typename T>
	T get_bigger(T first, T second)
	{
		if (first >= second)
		{
			return first;
		}
		else
			return second;
	}

	template<typename T>
	T get_closer(T first, T second, T number)
	{
		if (absolute(number - first) < absolute(number - second))
		{
			return first;
		}
		else
			return second;
	}

};