#pragma once

#include <filesystem>



namespace AppManagerChild
{
	void Init();
	void ShutDown();

	void OnUpdate();

	bool IsChessFileAvail(const std::filesystem::path& path);
	void OpenChessFile(const std::filesystem::path& path = "");
	void OwnChessFile(const std::filesystem::path& path);
}