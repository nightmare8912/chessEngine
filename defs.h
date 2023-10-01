#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>

// #define DEBUG

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
#define MAXPOSITIONMOVES 256 // max posible moves at a given position
#define MAXDEPTH 64

#define INFINITE 30000
#define MATE 29000
#define ISMATE (INFINITE - MAXDEPTH)

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


enum {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE};
enum {WHITE, BLACK, BOTH};
enum {UCIMODE, XBOARDMODE, CONSOLEMODE};

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

enum {FALSE, TRUE};

// castling permissions
// 0 0 0 0 (1st zero -> WKCA, 2nd zero -> WQCA, 3rd zero -> BKCA, 4th zero -> BQCA) 0->castling not allowed, 2->castling allowed
enum {WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

typedef struct {
	int move; // 1 2 4 8 16 32 64, using these 7 bits, we can represent any square on board (eg. 98 = 64 + 32 + 2), so we can represent any move using 7 bits
	// also, we'll be using hexadecimal to store these move values

	// 0000 0000 0000 0000 0000 0111 1111 -> the 7 least significant bits will be used to store the from pos (0x7F)
	// 0000 0000 0000 0011 1111 1000 0000 -> the next 7 least significant bits will be used to store the to pos -> to get the to square, we can shift 7 bits to the right ( >> 7[right shift by 7]) --> after shift --> (0x7F)
	// 0000 0000 0011 1100 0000 0000 0000 -> since our pieces go upto 12 (1 2 4 8), we can assign next 4 bits to store a captured piece (>> 14) --> after shift --> (0xF)
	// 0000 0000 0100 0000 0000 0000 0000 -> this bit will show an enpassant capture (0x40000)
	// 0000 0000 1000 0000 0000 0000 0000 -> this bit will show a pawn start (0x80000)
	// 0000 1111 0000 0000 0000 0000 0000 -> this bit will indicate a promotion piece (>> 20) , 0xF
	// 0001 0000 0000 0000 0000 0000 0000 -> this bit will indicate a castle move (0x1000000)

	int score; // used for move ordering
} S_MOVE;

typedef struct {
	S_MOVE moves[MAXPOSITIONMOVES];
	int count; // no of moves in that given position
} S_MOVELIST;

enum {HFNONE, HFALPHA, HFBETA, HFEXACT};

typedef struct {
	U64 posKey;
	int move;
	int score;
	int depth;
	int flags;
} S_HASHENTRY;

typedef struct {
	S_HASHENTRY *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} S_HASHTABLE;

typedef struct {
	U64 posKey;
	int move;
} S_PVENTRY;

typedef struct {
	S_PVENTRY *pTable;
	int numEntries;
} S_PVTABLE;	

// for history of the game
typedef struct {

	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;
	U64 posKey;

} S_UNDO;

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
	int bigPce[2]; // non pawn pieces
	int majPce[2]; // major pieces : rooks and queens
	int minPce[2]; // minor pieces : bishops and knights
	int material[2]; // material score : value of material for black and white

	S_UNDO history[MAXGAMEMOVES]; // will help us store every thing about the game at that state
	
	// piece list
	int pList[13][10]; // 13 representing types of piece and 10 for extreme cases(eg 2 rook + 8 pawns that all promote to rook)
	
	S_PVTABLE PvTable[1]; // pointer(the table)
	int PvArray[MAXDEPTH];

	int searchHistory[13][BRD_SQ_NUM]; // every time a move improves on alpha
	int searchKillers[2][MAXDEPTH]; // moves that caused a beta cutoff

	S_HASHTABLE HashTable[1];
	
} S_BOARD;

typedef struct {

	int startTime;
	int stopTime;
	int depth;
	int depthSet;
	int timeSet;
	int movesToGo;
	int infinite; // if true, search won't be stopped until gui sends stop command

	long nodes;

	int quit;
	int stopped;

	float fh; // fail high
	float fhf;  // fail high first

	int GAME_MODE;
	int POST_THINKING;
} S_SEARCHINFO;

typedef struct {
	int UseBook;
} S_OPTIONS;

/* MACROS */

// converts the given file and rank to corresponding square
#define FR2SQ(f, r) ( (21 + (f) ) + ( (r) * 10) ) // f->file, r->rank
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define POP(b) PopBit(b) // pos the bit from give ULL and returns it
#define CNT(b) CountBits(b)
#define CLRBIT(bb, sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

#define MIRROR64(sq) (Mirror64[(sq)])

#define IsBQ(p) (PieceBishopQueen[(p)]) // is the piece a bishop or a queen
#define IsRQ(p) (PieceRookQueen[(p)]) // is the piece a rook or a queen
#define IsKn(p) (PieceKnight[(p)]) // is the piece a Knight
#define IsKi(p) (PieceKing[(p)]) // is the piece a king

/* MACROS THAT HELP DECODE GAME MOVE */

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m) >> 7) & 0x7F)
#define CAPTURED(m) (((m) >> 14) &0xF)
#define PROMOTED(m) (((m) >> 20) & 0xF)
#define MFLAGEP 0x40000 // enpassant
#define MFLAGPS 0x80000 // pawn start
#define MFLAGCA 0x1000000 // castling
#define MFLAGCAP 0x7C000 // captures in that move -> 0000 0000 0111 1100 0000 0000 0000 (we've to include en-passant) -> corresponding hexadecimal ->0x7C000
#define MFLAGPROM 0xF00000

#define NOMOVE 0

/* GLOBALS */

extern int Sq120ToSq64[BRD_SQ_NUM]; // to convert our 120 sized board to standard 64 sized board
extern int Sq64ToSq120[64]; // opposite of what the other does
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int PieceBig[13];
extern int PieceMaj[13];
extern int PieceMin[13];
extern int PieceVal[13];
extern int PieceCol[13];

extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

extern int PiecePawn[13];
extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int PieceSlides[13];

extern U64 FileBBMask[8];
extern U64 RankBBMask[8]; // bb = bitboard	

extern U64 BlackPassedMask[64];
extern U64 WhitePassedMask[64];
extern U64 IsolatedMask[64];

extern S_OPTIONS EngineOptions[1];

/* FUNCTIONS */

// init.c
extern void AllInit();

// bitboards.c
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);

// hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

// board.c
extern void ResetBoard(S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void UpdateListsMaterial(S_BOARD *pos);
extern int CheckBoard(const S_BOARD *pos);
extern void MirrorBoard(S_BOARD *pos);

// attack.c
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);

// io.c
extern char *PrMove(const int move);
extern char *PrSq(const int sq);
extern void PrintMoveList(const S_MOVELIST *list);
extern int ParseMove(char *ptrChar, S_BOARD *pos);

// validate.c
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);
extern int MoveListOk(const S_MOVELIST *list,  const S_BOARD *pos);
extern void MirrorEvalTest(S_BOARD *pos);
extern void DebugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info);

// movegen.c
extern void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern int MoveExists(S_BOARD *pos, const int move);
extern void InitMvvLva();

// makemove.c
extern void TakeMove(S_BOARD *pos);
extern int MakeMove(S_BOARD *pos, int move);
extern void TakeNullMove(S_BOARD *pos);
extern void MakeNullMove(S_BOARD *pos);

// perft.c
extern void PerftTest(int depth, S_BOARD *pos);

// search.c
extern 	void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info);
// extern int IsRepitition(const S_BOARD *pos);

// misc.c
// extern void SearchPosition(S_BOARD *pos);
extern int GetTimeMs();
extern void ReadInput(S_SEARCHINFO *info);

// pvtable.c
extern void InitPvTable(S_PVTABLE *table);
extern void StorePvMove(const S_BOARD *pos, const int move);
int ProbePvMove(const S_BOARD *pos);
extern int GetPvLine(const int depth, S_BOARD *pos);
extern void ClearPvTable(S_PVTABLE *table);
extern void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth);
extern int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth);
extern void InitHashTable(S_HASHTABLE *table, int sizeInMb);
extern void ClearHashTable(S_HASHTABLE *table);
// evaluate.c
extern int EvalPosition(const S_BOARD *pos);
extern int Mirror64[64];

// uci.c
extern void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info);

// xboard.c
extern void XBOARD_Loop(S_BOARD *pos, S_SEARCHINFO *info);
extern void Console_Loop(S_BOARD *pos, S_SEARCHINFO *info);

// polybook.c
// extern U64 PolyKeyFromBoard(const S_BOARD *board);
extern void InitPolyBook();
extern void ClearPolyBook();
extern int GetBookMove(S_BOARD *board);

#endif