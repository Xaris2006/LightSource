#pragma once

#include <iostream>

#include "../math_Lib.h"


#define TESTING() 

template<typename T>
class vec2
{
public:
	vec2() = default;
	inline vec2(T xcoord, T ycoord)
		: c{ xcoord , ycoord }
	{

	}
	~vec2() = default;

	inline T& x()
	{
		TESTING();
		return c[0];
	}
	inline T& y()
	{
		TESTING();
		return c[1];
	}

	inline const T x() const
	{
		TESTING();
		return c[0];
	}
	inline const T y() const
	{
		TESTING();
		return c[1];
	}
	
	inline vec2 operator-() const
	{
		TESTING();
		return vec2(-c[0], -c[1]);
	}

	inline vec2 operator+(const vec2& first) const
	{
		TESTING();
		return vec2(c[0] + first.c[0], c[1] + first.c[1]);
	}
	inline vec2 operator-(const vec2& first) const
	{
		TESTING();
		return vec2(c[0] - first.c[0], c[1] - first.c[1]);
	}
	inline vec2 operator*(const vec2& first) const
	{
		TESTING();
		return vec2(c[0] * first.c[0], c[1] * first.c[1]);
	}
	inline vec2 operator/(const vec2& first) const
	{
		TESTING();
		float factor_x;
		float factor_y;
		if (first.c[0] == 0)
		{
			factor_x = 0;
		}
		else
		{
			factor_x = c[0] / first.c[0];
		}

		if (first.c[1] == 0)
		{
			factor_y = 0;
		}
		else
		{
			factor_y = c[1] / first.c[1];
		}

		TESTING();
		return vec2(factor_x, factor_y);
	}

	inline bool operator==(const vec2& first)
	{
		TESTING();
		if (c[0] == first.c[0] && c[1] == first.c[1])
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	inline bool operator!=(const vec2& first)
	{
		TESTING();
		if (c[0] == first.c[0] && c[1] == first.c[1])
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	inline bool operator<=(const vec2& first)
	{
		TESTING();
		if (c[0] > first.c[0] || c[1] > first.c[1])
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	inline bool operator>=(const vec2& first)
	{
		TESTING();
		if (c[0] < first.c[0] || c[1] < first.c[1])
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	inline bool operator>=(const T& number)
	{
		TESTING();
		if (c[0] < number || c[1] < number)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	inline bool operator<(const vec2& first)
	{
		TESTING();
		if (c[0] < first.c[0] || c[1] < first.c[1])
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	inline operator vec2<uint8_t>()
	{
		TESTING();
		return vec2<uint8_t>((uint8_t)c[0], (uint8_t)c[1]);
	}
	
	inline friend std::ostream& operator<<(std::ostream& stream, const vec2& object)
	{
		stream << object.c[0] << ' ' << object.c[1];
		return stream;
	}
	inline friend std::istream& operator>>(std::istream& stream, vec2& object)
	{
		stream >> object.c[0] >> object.c[1];
		return stream;
	}

	inline static vec2 absolute(const vec2& number)
	{
		return vec2(mtcs::absolute(number.c[0]), mtcs::absolute(number.c[1]));
	}

private:
	T c[2];
};