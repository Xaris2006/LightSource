#include "AppManagerChild.h"

#include "Walnut/Application.h"

#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

namespace AppManagerChild
{
	void Init()
	{

#ifndef WL_DIST

		return;

#endif // !WL_DIST

		std::atomic<bool> ParentExists = false;

		std::cerr << "App: " << std::this_thread::get_id() << " - *Start\n";
		std::thread CheckParent(
			[&ParentExists]()
			{
				std::string input;
				std::cin >> input;
				if (input == "Ok")
					ParentExists = true;
			});
		

		for (int i = 0; i < 15; i++)
		{
			using namespace std::chrono_literals;

			std::this_thread::sleep_for(100ms);

			if (ParentExists)
				break;
		}

		if (!ParentExists)
			Walnut::Application::Get().Close();
		else
			CheckParent.join();
	}

	void ShutDown()
	{

#ifndef WL_DIST

		return;

#endif // !WL_DIST

		std::cerr << "App: " << std::this_thread::get_id() << " - *End\n";
	}

	void OnUpdate()
	{

#ifndef WL_DIST

		return;

#endif // !WL_DIST

		std::cerr << "App: " << std::this_thread::get_id() << " \n";// << " - *File Path:" << ChessAPI::GetPgnFilePath() << ":Path \n";
	}

	bool IsChessFileAvail(const std::filesystem::path& path)
	{

#ifndef WL_DIST

		return true;

#endif // !WL_DIST

		std::cerr << "App: " << std::this_thread::get_id() << " - *Ask Path:" << path << ":Path \n";
		std::string anwser;
		std::cin >> anwser;
		if (anwser == "Accept")
			return true;
		else if (anwser == "Decline")
			return false;

		return false;
	}

	void OpenChessFile(const std::filesystem::path& path)
	{

#ifndef WL_DIST

		return;

#endif // !WL_DIST


		if (path == "")
		{
			std::cerr << "App: " << std::this_thread::get_id() << " - *Open\n";
			return;
		}

		std::cerr << "App: " << std::this_thread::get_id() << " - *Open Path:" << path << ":Path \n";
	}

	void OwnChessFile(const std::filesystem::path& path)
	{

#ifndef WL_DIST

		return;

#endif // !WL_DIST

		std::cerr << "App: " << std::this_thread::get_id() << " - *File Path:" << path << ":Path \n";
	}
}