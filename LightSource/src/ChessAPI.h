#pragma once

#include <string>

#include "glm/glm.hpp"

#include "ChessCore/vec2/vec2_lib.h"
#include "ChessCore/pgn/Pgn.h"
#include "ChessCore/Opening_Book/OpeningBook.h"

namespace ChessAPI
{
	void Init();

	chess::Pgn_File* GetPgnFile();
	chess::Pgn_Game* GetPgnGame();
	std::string& GetPgnFilePath();
	std::string& GetPgnFileName();

	std::vector<std::string>	GetCurPgnLabelNames();
	std::string&				GetCurPgnLabelValue(const std::string& name);
	void						GetMovesPgnFormat(chess::Pgn_Game::ChessMovesPath& moves);
	std::vector<int>&			GetMoveIntFormat();
	std::string&				GetNote(const std::vector<int>& pathmove);
	
	std::string						GetPieceName(int type);
	int								GetBlockID(int BlockIndex);
	
	bool							GetPlayerColor();
	std::vector<vec2<float>>		GetPossibleDirections(const vec2<float>& pos);
	std::string						GetFEN();
	chess::OpeningBook::PositionID	GetFormatedPosition();



	void SetNewChessGame(const std::string& path);
	void SetNewChessGame(int index);
	void OverWriteChessFile(const std::string& filepath);
	void NewGameInFile();
	void DeleteGameInFile(int index);

	void SetNotePanelPointed(const bool& ispointed);

	void GoMoveByIntFormat(std::vector<int>& pathmoves);
	void GoMoveByStr(const std::string& movename);
	void NextSavedMove();
	void PreviousSavedMove();

	bool IsMoveValid(glm::vec2 pos, glm::vec2 dir);
	bool IsBoardChanged();
	bool IsNewVariationAdded();

	void DeleteMove(std::vector<int>& movepath);
	void DeleteVariation(std::vector<int>& movepath);
	void PromoteVariation(std::vector<int>& movepath);

	void SetNewPieceType(int type);
	bool IsWaitingForNewType();

	//void ReverseBoard();
	//bool GetReverseBoardType();

}