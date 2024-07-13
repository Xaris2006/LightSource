#pragma once

#include <vector>
#include <string>

#include "Walnut/Image.h"
#include "ChessCore/pgn/Pgn.h"

#include "ChessAPI.h"

namespace Panels {

	class MovePanel
	{
	public:
		MovePanel() = default;

		void OnImGuiRender();

		bool& IsPanelOpen();

	private:
		void WriteMove(const chess::Pgn_Game::ChessMovesPath& par, std::vector<int>& pathmove, float extrain = 0);


	private:
		bool m_viewPanel = true;

		chess::Pgn_Game::ChessMovesPath m_moves;
	};
}
