#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include "Walnut/Image.h"

namespace Panels
{
	class ToolsPanel
	{
	public:
		ToolsPanel();

		void OnImGuiRender();

	private:
		void FindAvailableTools();
		void FindDownloadableTools(); 

	private:

		std::vector<std::filesystem::path> m_AvailableTools;
		std::vector<std::filesystem::path> m_DownloadableTools;

		int m_TargetedToolIndex = -1;

		std::unordered_map<std::string, std::string> m_ToolLabelNameToValue;

		std::vector<std::shared_ptr<Walnut::Image>> m_ToolIcons;
	};

}