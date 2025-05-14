#pragma once

#include <string>

#include "../../Walnut/vendor/glm/glm/glm.hpp"

#include "ChessCore/GameManager.h"
#include "ChessCore/pgn/Pgn.h"
#include "ChessCore/Opening_Book/OpeningBook.h"

namespace ChessAPI
{
	void Init();

	int GetActiveGame();
	std::vector<int>& GetOpenGames();
	bool IsGameOpen(size_t index);
	void CloseOpenGame(int index);

	Chess::PgnFile* GetPgnFile();
	Chess::PgnGame* GetPgnGame();
	std::string& GetPgnFilePath();
	std::string& GetPgnFileName();

	std::vector<std::string>		GetCurPgnLabelNames();
	std::string&					GetCurPgnLabelValue(const std::string& name);
	void							GetMovesPgnFormat(Chess::PgnGame::ChessMovesPath& moves);
	std::vector<int>&				GetMoveIntFormat();
	std::string&					GetNote(const std::vector<int>& pathmove);
	
	std::string						GetPieceName(int type);
	int								GetBlockID(int BlockIndex);
	
	bool							GetPlayerColor();
	void							GetPossibleDirections(int pos, std::vector<Chess::Board::Move>& moves);
	std::string						GetFEN();
	Chess::OpeningBook::PositionID	GetFormatedPosition();



	void OpenChessFile(const std::string& path);
	void OpenChessGameInFile(int index);
	void OverWriteChessFile(const std::string& filepath);
	void NewGameInFile();
	//Should not be used
	void DeleteGamesInFile();

	void SetNotePanelPointed(const bool& ispointed);

	void GoMoveByIntFormat(std::vector<int>& pathmoves);
	void GoMoveByStr(const std::string& movename);
	void NextSavedMove();
	void PreviousSavedMove();

	bool IsMoveValid(glm::vec2 pos, glm::vec2 dir);
	bool MakeMove(glm::vec2 pos, glm::vec2 dir);
	bool IsBoardChanged();

	void DeleteMove(std::vector<int>& movepath);
	void DeleteVariation(std::vector<int>& movepath);
	void PromoteVariation(std::vector<int>& movepath);

	void SetNewPieceType(int type);
	bool IsWaitingForNewType();
}