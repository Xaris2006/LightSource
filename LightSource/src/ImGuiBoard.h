#pragma once

#include "imgui.h"
#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include <iostream>
#include <array>


class ImGuiBoard
{
public:
	void OnAttach();

	void OnUIRender();

	void FlipBoard();

private:

	void RenderBoard();
	void RenderPieces();
	ImVec2 FindMousePos();

	void  UpdateBoardValues();

private:
	float m_size;
	ImVec2 m_startCursor;

	int m_CapturedPieceIndex = 0;
	float m_oldNumX = -1, m_oldNumY = -1;

	bool m_AboutModalOpen = false;

	bool m_NextMove = false;

	bool m_reverse = false;
	std::shared_ptr<Walnut::Image> m_board[2];
	std::shared_ptr<Walnut::Image> m_pieces[12];

	std::array<std::array<int, 8>, 8> m_block;
};
