// init.c

#include "defs.h"
#include <stdlib.h>
 
// 0000 000000000000000 000000000000000 000000000000000 000000000000000
#define RAND_64 		((U64)rand() | \
						(U64)rand() << 15 | \
						(U64)rand() << 30 | \
						(U64)rand() << 45 | \
						((U64)rand() & 0xf) << 60 ) 

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM]; // these two arrays will store the value of the files and rank for corresponding array 120 index

U64 FileBBMask[8];
U64 RankBBMask[8]; // bb = bitboard	

U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];

/*
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 x 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0 

	to check if x is passed pawn -> we take following bit board

	0 0 0 1 1 1 0 0
	0 0 0 1 1 1 0 0
	0 0 0 1 1 1 0 0
	0 0 0 1 1 1 0 0
	0 0 0 1 1 1 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0 

	now we perform and between them, and if result is zero-> it is a passed pawn

	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0
	0 0 0 0 0 0 0 0 

*/

S_OPTIONS EngineOptions[1];

// this function sets up the rows and columns of bitboards to 1's so that we can use them to detect passed pawns at a later stage
void InitEvalMasks() {
	
	int sq, tsq, r, f;

	for (sq = 0; sq < 8; sq++) {
		FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	for (r = RANK_8; r >= RANK_1; r--) {
		for (f = FILE_A; f <= FILE_H; f++) {
			sq = r * 8 + f;
			FileBBMask[f] |= (1ULL << sq);
			RankBBMask[r] |= (1ULL << sq);
		}
	}

	for (sq = 0; sq < 64; sq++) {
		IsolatedMask[sq] = 0ULL;
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
	}

	for (sq = 0; sq < 64; sq++) {
		// incrementing by 8 -> (for white pawn) means staying in same file but incrementing the rank by 1
		tsq = sq + 8;
		while (tsq < 64) {
			// setting all the positions ahead of the start square to x(1)
			WhitePassedMask[sq] |= (1ULL << tsq);
			tsq += 8;
		}
		// decrementing by 8 -> (for black pawn) means staying in same file but decrementing the rank by 1
		tsq = sq - 8;
		while (tsq >= 0) {
			// setting all the positions ahead of the start square to x(1)
			BlackPassedMask[sq] |= (1ULL << tsq);
			tsq -= 8;
		}

		if (FilesBrd[SQ120(sq)] > FILE_A) {
			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1]; // self explanatory -> we just take the adjacent file and set it all to x

			tsq = sq + 7; // incrementing by 7 means that we're going to the adjacent file
			while (tsq < 64) {
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8; // now since we're alredy on the adjacent file, we have to increment by 8 not 7 (as we want to stay on the same adjacent file, and not change the file anymore)
			}

			tsq = sq - 9;
			while (tsq >= 0) {
				BlackPassedMask[sq] |= (1ULL << tsq);	
				tsq -= 8;
			}
		}

		if (FilesBrd[SQ120(sq)] < FILE_H) {
			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];
			tsq = sq + 9; // incrementing by 9 means that we're going to the adjacent file
			while (tsq < 64) {
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8; // now since we're alredy on the adjacent file, we have to increment by 8 not 7 (as we want to stay on the same adjacent file, and not change the file anymore)
			}

			tsq = sq - 7;
			while (tsq >= 0) {
				BlackPassedMask[sq] |= (1ULL << tsq);
				tsq -= 8;
			}
		}
	}
	// for (sq = 0; sq < 64; ++sq) {
	// 	PrintBitBoard(IsolatedMask[sq]);
	// }
}

void InitFilesRanksBrd()
{
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;

	for (index = 0; index < BRD_SQ_NUM; ++index)
	{
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	} 

	for (rank = RANK_1; rank <= RANK_8; ++rank)
	{
		for (file = FILE_A; file <= FILE_H; ++file)
		{
			sq = FR2SQ(file, rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}
}

void InitHashKeys()
{
	int index = 0;
	int index2 = 0;
	for (index = 0; index < 13; ++index)
	{
		for (index2 = 0; index2 < 120; ++index2)
		{
			PieceKeys[index][index2] = RAND_64;
		}
	}
	SideKey = RAND_64;
	for (index = 0; index < 16; ++index)
	{
		CastleKeys[index] = RAND_64;
	}
}

void InitBitMask()
{
	int index = 0;

	for (index = 0; index < 64; index++)
	{
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for (index = 0; index < 64; index++)
	{
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index];
	}
}

void InitSq120To64()
{
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;
	
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65;
	}
	
	for (index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120;
	}
	
	for (rank = RANK_1; rank <= RANK_8; ++rank) {
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file, rank);
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			sq64++;
		}
	}
}

void AllInit()
{
	InitSq120To64();
	InitBitMask();
	InitHashKeys();
	InitFilesRanksBrd();
	InitEvalMasks();
	InitMvvLva();
	InitPolyBook();
}