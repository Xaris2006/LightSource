#pragma once

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <atomic>

#include "Walnut/Image.h"

namespace Panels {

	class EnginePanel
	{
	public:
		EnginePanel();
		~EnginePanel();

		void OnImGuiRender();
		void Reset();

		std::string GetDefaultEngine() const;

		void OpenEngine(const std::string& programpath);
		void CloseEngine();
		bool IsEngineOpen() const;
		std::vector<std::string>& GetAvailEngines();
	private:
		void CommandChessEngine(const std::string& command);

		void GetBestMoveStr(int list, std::vector<std::string>& moves) const;
		int						 GetDepth() const;
		int						 GetNodesPerSecond() const;
		std::string				 GetName();

	private:
		bool m_viewPanel = false;
		bool m_running = false;

		std::vector<uint8_t> m_oldBoard;

		int m_lines = 1;
		int m_maxLines = 5;
			
		int m_threadCount = 2;
		int m_hashMb = 256;

		int m_SkillLevel = 20; //0->20

		std::string m_SyzygyPath = "";
		bool m_Syzygy50MoveRule = true;

		bool m_LimitStrength = false;
		int m_Elo = 1400;

		std::string m_DefaultEngine = "";
		std::vector<std::string> m_AvailEngines;

		std::shared_ptr<Walnut::Image> m_IconPlay = std::make_shared<Walnut::Image>("Resources/Icons/PlayButton.png");
		std::shared_ptr<Walnut::Image> m_IconStop = std::make_shared<Walnut::Image>("Resources/Icons/StopButton.png");

	private:
		std::thread* m_processThread = nullptr;

		std::vector<std::string> m_write;

		std::atomic<bool> m_EndThread = true;
		bool m_BlackToPlay = false;

		int m_Depth = 404;
		int m_Nps = 404;
		float m_Score[5];
		std::vector<std::string> m_Moves[5];
		int m_FinalStreams[5];

		std::string m_EngineName;
	};
}


