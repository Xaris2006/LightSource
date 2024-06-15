#pragma once

#include <vector>
#include <string>

#include "Hazel/Renderer/Texture.h"

#include "chess.h"

namespace Hazel {

	class EnginePanel
	{
	public:
		EnginePanel(ChessAPI::ChessAPI* chess);

		void OnImGuiRender();
		void Reset();

		void SetDefaultEngine(const std::string& path);
		std::string GetDefaultEngine() const;

	private:
		bool m_viewPanel = false;
		bool m_running = true;

		ChessAPI::ChessAPI* m_chess;

		std::vector<uint8_t> m_oldBoard;

		int m_lines = 1;
		int m_maxLines = 5;
			
		int m_threadCount = 2;
		int m_hashMb = 256;

		float m_winner = 1;

		Ref<Texture2D> m_IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
		Ref<Texture2D> m_IconStop = Texture2D::Create("Resources/Icons/StopButton.png");

		std::string m_defaultEngine = "ChessProject\\Assets\\engines\\stockfish.exe";
	};
}


