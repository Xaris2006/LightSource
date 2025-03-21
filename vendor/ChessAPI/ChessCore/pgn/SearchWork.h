#pragma once

#include "PgnManager.h"

namespace Chess
{
	class SearchWork
	{
	public:
		SearchWork();
		~SearchWork();

		void SearchLabel(PgnManager::FileID id, const std::map<std::string, std::string>& settings);
		
		void SearchMoves(PgnManager::FileID id, const std::vector<std::string>& moves);

		void Clear();

		//only read
		std::vector<size_t>& GetResult() const;

		float GetPercentage() const;

	private:
		PgnManager::WorkID m_ID;

		std::shared_ptr<PgnManager::SearchWorkData> m_Data;
	};
}