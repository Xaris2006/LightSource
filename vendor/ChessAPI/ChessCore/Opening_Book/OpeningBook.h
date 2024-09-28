#pragma once

#include <string>
#include <vector>
#include <map>

///COB file format
// 
// ----Version----
// 
// (char)byte write: COB_FORMAT_VERSION_0.1
// 
// ----End_Version----
// 
// ----Position----
// 
// c : color
// t : type
// p : position
// r : roke
// 
// board:
// 
// 0b76543210
// --ctttcttt : t-> 0(Pawn), 1(Knight), 2(Bishop), 3(Rook), 4(Queen), 5(King)
// 
// 0b76543210 
// --0110----  : 0,6-> 1 clear 
// --1110----  : 1,6-> 2 clear
// --0111----  : 0,7-> 3 clear
// --1111----  : 1,7-> 4 clear
// 
// ----------
// 
// settings:
// 
// 0b76543210
// --cppprrrr : 7->player to play, 6,5,4->an pan san position, 3,2,1,0-> qkQK
//
// ----End_Position----
// 
// ----Moves----
// 
// array of:
// {
//		array of char strmove (uknown bytes)
//		0                     (1 byte 0b00000000)
//		int whitewins		 (4 bytes)
//		int blackwins		 (4 bytes)
//		int played			 (4 bytes)
// }
// 
// ----End_Moves----



namespace chess
{
	class OpeningBook
	{
	public:
		struct MoveOB
		{
			std::string strmove = "";
			uint32_t whitewins = 0;
			uint32_t blackwins = 0;
			uint32_t played = 0;

			void operator= (const MoveOB& other)
			{
				strmove = other.strmove;
				whitewins = other.whitewins;
				blackwins = other.blackwins;
				played = other.played;
			}
		};

		using PositionID = std::vector<uint8_t>;

		struct PositionID_Hash
		{
			size_t operator() (const PositionID& pos) const
			{
				std::size_t hash = 0;
				std::hash<uint8_t> hasher;
				for (uint8_t byte : pos)
				{
					// Combine the hash values (simple XOR + shift approach)
					hash ^= hasher(byte) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
				}
				return hash;
			}
		};

		enum SortingType
		{
			RAW = 0,
			MOSTPLAYED,
			BESTWHITE,
			BESTBLACK,
			BESTDRAW
		};

	public:
		static void CreateCOBByPGN(const std::string& pgnfilepath, int& status);
	public:
		OpeningBook(const std::string& cobfilepath);
		~OpeningBook() = default;

		//You should use the function as it was copying the data (Just trying to be faster)
		std::vector<MoveOB>& GetMovesByPosition(const PositionID& posID, const SortingType& stype = RAW);


	private:
		OpeningBook();

	private:
		std::string m_version;

		std::vector<uint8_t> m_data;
		size_t m_size = 0;
		std::vector<MoveOB> m_lastMoveOB;
		std::vector<MoveOB> m_SortedlastMoveOB;
		PositionID m_lasePosID;
		SortingType m_lastSortingType;
	};

}
