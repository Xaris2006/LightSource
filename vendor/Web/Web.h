#pragma once

#include <string>

namespace Web
{
	enum DownLoadStatus
	{
		Nothing = 0,
		Starting,
		DownLoading,
		Finished,
		Error
	};

	void DownLoadFile(const std::string& server, const std::string& url, const std::string& savePath, DownLoadStatus& status);

	std::string DownLoadFileFromGoogleDrive(const std::string& id, const std::string& at, DownLoadStatus& status);
}