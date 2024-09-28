#pragma once

#include "Walnut/Application.h"

#include "Panels/ContentBrowserPanel.h"

namespace Tools::COBCreator
{
	class Layer : public Walnut::Layer
	{
	public:
		virtual void OnAttach() override;

		virtual void OnDetach() override;

		virtual void OnUIRender() override;

		void ShowAboutModal();

		void AddFile();

	private:
		void UI_DrawAboutModal();

	private:
		bool m_AboutModalOpen = false;

		int depth = 40;

		std::shared_ptr<Walnut::Image> m_PlayIcon;
		std::shared_ptr<Walnut::Image> m_StopIcon;
		std::shared_ptr<Walnut::Image> m_PauseIcon;
		std::shared_ptr<Walnut::Image> m_ResumeIcon;

		std::unique_ptr<Panels::ContentBrowserPanel> m_ContentBrowserPanel;
	};

	Walnut::Application* CreateApplication(int argc, char** argv);
}