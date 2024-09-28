#include "Web.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <filesystem>

namespace Web
{
	void DownLoadFile(const std::string& server, const std::string& url, const std::string& savePath, DownLoadStatus& status)
	{
		status = Starting;

		httplib::SSLClient cli(server);

		std::ofstream file(savePath, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open file for writing." << std::endl;
			return;
		}

		// Progress callback to write data as it's received
		auto progress = [&](const char* data, size_t data_length) {
			status = DownLoading;
			file.write(data, data_length);
			//std::cout << "Downloaded " << data_length << " bytes." << std::endl;
			return true;
			};

		// Make initial GET request
		auto res = cli.Get(url.c_str(), progress);
		
		//status = DownLoading;

		// Check for HTTP 303 redirection
		if (res && res->status == 303) {
			std::string redirect_url = res->get_header_value("Location");
			std::cout << "Redirecting to: " << redirect_url << std::endl;

			// Extract the new host and path from the redirect URL
			httplib::SSLClient redirect_cli(server);

			auto res_redirect = redirect_cli.Get(redirect_url.c_str(), progress);

			if (res_redirect && res_redirect->status == 200) {
				//file.write(res_redirect->body.c_str(), res_redirect->body.size());
				status = Finished;
				std::cout << "File downloaded successfully after redirection." << std::endl;
			}
			else {
				status = Error;
				std::cerr << "Failed to download file after redirection. HTTP status: "
					<< (res_redirect ? res_redirect->status : 0) << std::endl;
			}
		}
		else if (res && res->status == 200) {
			//file.write(res->body.c_str(), res->body.size());
			status = Finished;
			std::cout << "File downloaded successfully." << std::endl;
		}
		else {
			status = Error;
			std::cerr << "Failed to download file. HTTP status: " << (res ? res->status : 0) << std::endl;
		}

		file.close();
	}

	std::string DownLoadFileFromGoogleDrive(const std::string& id, const std::string& at, DownLoadStatus& status)
	{
		status = Starting;

		httplib::SSLClient cli("drive.usercontent.google.com");

		std::ofstream file("download" + id, std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "Failed to open file for writing." << std::endl;
			status = Error;
			return "";
		}

		// Progress callback to write data as it's received
		auto progress = [&](const char* data, size_t data_length) {
			status = DownLoading;
			file.write(data, data_length);
			//std::cout << "Downloaded " << data_length << " bytes." << std::endl;
			return true;
			};

		std::string url = "/download?id=" + id + "&export=download&confirm=t&at=" + at;

		// Make initial GET request
		auto res = cli.Get(url.c_str(), progress);

		if (res && res->status == 200) {
			status = Finished;
			std::cout << "File downloaded successfully." << std::endl;
		}
		else 
		{
			status = Error;
			std::cerr << "Failed to download file. HTTP status: " << (res ? res->status : 0) << std::endl;
		}

		file.close();

		if (status == Error)
		{
			std::filesystem::remove_all("download" + id);
			return "";
		}

		std::string contentdisposition = res->get_header_value("content-disposition");
		std::string filename(contentdisposition.begin() + contentdisposition.find("=") + 2, contentdisposition.end() - 1);

		std::error_code ec;
		std::filesystem::rename("download" + id, filename, ec);

		return filename;
	}
}