#pragma once

#include <string>

namespace Windows::Utils
{
	std::string OpenFile(const char* Filter);

	std::string SaveFile(const char* Filter);
}