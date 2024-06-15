#pragma once

#include <array>
#include <vector>
#include <map>
#include <string>

#include "../ChessAPI.h"

namespace Panels {

	class DatabasePanel
	{
	public:
		DatabasePanel();

		void Reset();

		void OnImGuiRender();

	private:

		std::array<std::pair<float, float>, 7> m_columnsSettings;
		std::map<std::string, std::string> m_searchSetting;
		bool m_name_white = true;
		bool m_name_black = true;
		std::string m_name_to_search;
		std::string m_eco_to_search;
		std::vector<std::string> m_ecoItems;
		std::string m_date_to_search;
		std::vector<int> m_search_resualt;
		int m_lastPointedRow = 0;

		std::vector<std::string> m_important_prop = {
			"White",    "Black",
			"WhiteElo", "BlackElo",
			"Date", "ECO", "Result",
			"Event", "Round", "Site"
		};
	};

	

}
