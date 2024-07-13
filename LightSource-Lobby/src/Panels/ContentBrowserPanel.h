#pragma once

#include "ChessAPI.h"

#include "Walnut/Image.h"

#include <filesystem>
#include <memory>

namespace Panels
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
	private:

		void TreeDirectory(const std::filesystem::path& directory);

		void FilePopup();
		void EmptyPopup();
		void RenamePopup();
		void NewPopup();

	private:

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;

		std::shared_ptr<Walnut::Image> m_DirectoryIcon;
		std::shared_ptr<Walnut::Image> m_FileIcon;
		std::shared_ptr<Walnut::Image> m_FileIconPGN;
		std::shared_ptr<Walnut::Image> m_FileIconCOB;
		std::shared_ptr<Walnut::Image> m_BackArrow;
	};

}
