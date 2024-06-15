#pragma once

#include <vector>
#include <string>

//#include "chess.h"

namespace Hazel {

	class OpeningBookPanel
	{
	public:
		//OpeningBookPanel(ChessAPI::ChessAPI* chess);

		void OnImGuiRender();
		
		bool& IsPanelOpen();

	private:
		bool m_viewPanel = false;

		ChessAPI::ChessAPI* m_chess;
	};
}
