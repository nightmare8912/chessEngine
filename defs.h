#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>

#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if (!(n)) { \
printf("%s - FAILED ", #n); \
printf("On %s ", __DATE__); \
printf("At %s ", __TIME__); \
printf("In file %s ", __FILE__); \
printf("At line %d\n", __LINE__); \
exit(1) ; }
#endif
 

typedef unsigned long long U64;

#define NAME "Vince 1.0"
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048 // half moves


enum {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE};
enum {WHITE, BLACK, BOTH};

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

enum {FALSE, TRUE};

// castling permissions
// 0 0 0 0 (1st zero -> WKCA, 2nd zero -> WQCA, 3rd zero -> BKCA, 4th zero -> BQCA) 0->castling not allowed, 2->castling allowed
enum {WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

// for history of the game

typedef struct {

	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;
	U64 posKey;

}S_UNDO;

typedef struct {
	
	int pieces[BRD_SQ_NUM];
	U64 pawns[3]; // 3 because we've three colors, white, black and both 00100000(for sq a1 t0 h1 with pawn at b1) 00000000 00000000 00000000 00000000 
	// 0th index-> white pawn, 1st index-> black pawn, 2nd index->both pawns

	int KingSq[2]; // to store kings locn of both colors
	
	int side; // side to move
	int enPas; // enpassant square
	int fiftyMove; // fifty move with no captures means draw
	
	int ply; // to know how much half moves calculated in search
	int histPly; // to know how many total half moves made in the game
	
	int castlePerm;
	
	U64 posKey; // unique key generated for each position

	int pceNum[13]; // no of pieces we've on board, indexed by pieceType
	int bigPce[3]; // non pawn pieces
	int majPce[3]; // major pieces : rooks and queens
	int minPce[3]; // minor pieces : bishops and knights
	
	S_UNDO history[MAXGAMEMOVES]; // will help us store every thing about the game at that state
	
	// piece list
	int pList[13][10]; // 13 representing types of piece and 10 for extreme cases(eg 2 rook + 8 pawns that all promote to rook)
	
}S_BOARD;

/* MACROS */

// converts the given file and rank to corresponding square
#define FR2SQ(f, r) ( (21 + (f) ) + ( (r) * 10) ) // f->file, r->rank
#define SQ64(sq120) Sq120ToSq64[sq120]
#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)
#define CLRBIT(bb, sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb, sq) ((bb) != SetMask[(sq)])

/* GLOBALS */

extern int Sq120ToSq64[BRD_SQ_NUM]; // to convert our 120 sized board to standard 64 sized board
extern int Sq64ToSq120[64]; // opposite of what the other does
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];

/* FUNCTIONS */

// init.c
extern void AllInit();

// bitboards.c
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);

// hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

#endif