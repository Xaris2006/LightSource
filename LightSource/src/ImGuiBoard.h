#pragma once

#include "imgui.h"
#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include <iostream>
#include <array>
#include <map>
#include <vector>

class ImGuiBoard
{
public:
	void OnAttach();

	void OnUIRender();

	void FlipBoard();

	void OpenEditor();

private:

	void RenderBoard();
	void RenderPieces();
	void RenderCirclesAtPossibleMoves();
	ImVec2 FindMousePos();
	void  UpdateBoardValues();

private:
	void NextMovePopup();
	void NewVariantPopup();
	void NewPiecePopup();
	void EditorPopup();

private:
	float m_size;
	ImVec2 m_startCursor;

	int m_CapturedPieceIndex = 0;
	float m_oldNumX = -1, m_oldNumY = -1;

	bool m_reverse = false;
	std::shared_ptr<Walnut::Image> m_board[2];
	std::shared_ptr<Walnut::Image> m_pieces[12];
	std::shared_ptr<Walnut::Image> m_circleFromStart;
	std::shared_ptr<Walnut::Image> m_circleToEnd;
	std::shared_ptr<Walnut::Image> m_RedX;

	std::array<std::array<int, 8>, 8> m_block;
	
private:
	bool m_NextMove = false;
	ImVec2 m_Center;
	std::map<std::string, std::vector<int>> m_PossibleNextMoves;
	std::string m_MainMove;

private:
	bool m_ToOpenEditor = false;
	std::array<std::array<int, 8>, 8> m_Editorblock;
};
