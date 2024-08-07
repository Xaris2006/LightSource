#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>

#include "ChessCore/Opening_Book/OpeningBook.h"


namespace Panels {

	class OpeningBookPanel
	{
	public:
		OpeningBookPanel() = default;
		~OpeningBookPanel();

		void OnImGuiRender();
		
		void CreateCOBfile(const std::string& pgnfilepath);
		void OpenCOBfile(const std::string& filepath);
		bool CloseCOBfile();
		std::vector<chess::OpeningBook::MoveOB>& GetOpeningBookMoves(const chess::OpeningBook::PositionID& posID);

		int IsReadyCreateThread();

		bool& IsPanelOpen();

	private:
		bool m_viewPanel = true;

		chess::OpeningBook* m_OpeningBook = nullptr;
		chess::OpeningBook::PositionID m_CurPosition;
		std::vector<chess::OpeningBook::MoveOB> m_Moves;

		std::string m_cobPath = "";
		std::string m_cobFilename = "";

		std::thread* m_PlayThread = nullptr;
		std::atomic<bool> m_EndPlayThreadJob = true;

		std::thread* m_CreateThread = nullptr;
		std::atomic<bool> m_IsCreateThreadEnd = true;
		std::atomic<bool> m_EndCreateThreadJob = true;
		int m_Status = 0;
	};
}
