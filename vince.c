#include <stdio.h>
#include "defs.h"

#define FEN1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
#define FEN2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
#define FEN4 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define FEN5 "8/3q1p2/8/5P2/4Q3/8/8/8 w - - 0 2"

int main()
{
	AllInit();

	int move = 0;
	int from = A2, to = H7;
	int cap = wR, prom = wB;

	move = ((from) | (to << 7) | (cap << 14) | (prom << 20));
	printf("from %d, to %d, cap %d, prom %d\n", FROMSQ(move), TOSQ(move), CAPTURED(move), PROMOTED(move));

	printf("Algebraic from: %s\n", PrSq(from));
	printf("Algebraic to: %s\n", PrSq(to));
	printf("Algebraic move: %s\n", PrMove(move));

    return 0;
}

// ParseFen(START_FEN, board);
	// PrintBoard(board);

	// ParseFen(FEN1, board);
	// PrintBoard(board);

	// ParseFen(FEN2, board);
	// PrintBoard(board);

	// ParseFen(FEN3, board);
	// PrintBoard(board);

	// ParseFen(FEN4, board);
	// PrintBoard(board);

	// ASSERT(CheckBoard(board));

	// printf("hi");
	// AllInit();
	
	// S_BOARD board[1];

	// ParseFen(FEN5, board);
	// PrintBoard(board);

	// printf("\nWhite attacking: \n");
	// ShowSqAtBySide(WHITE, board);	

	// printf("\nBlack attacking: \n");
	// ShowSqAtBySide(BLACK, board);

// void ShowSqAtBySide(const int side, const S_BOARD *pos)
// {
// 	int rank = 0;
// 	int file = 0;
// 	int sq = 0;

// 	printf("\n\nSquares attacked by: %c\n", SideChar[side]);
// 	for (rank = RANK_8; rank >= RANK_1; --rank)
// 	{
// 		for (file = FILE_A; file <= FILE_H; ++file)
// 		{
// 			sq = FR2SQ(file, rank);
// 			if (SqAttacked(sq, side, pos) == TRUE)
// 			{
// 				printf("X");
// 			}
// 			else
// 			{
// 				printf("-");
// 			}
// 		}
// 		printf("\n");
// 	}
// 	printf("\n\n");
// }
 
// -----------------------------------------------

// void PrintBin(int move) {
// 	int index = 0;
// 	printf("As binary:\n");
// 	for(index = 27; index >= 0; index--) {
// 		if( (1 << index) & move) printf("1");
// 		else printf("0");
// 		if(index!=28 && index%4==0) printf(" ");
// 	}
// 	printf("\b");
// }

//  AllInit();

// 	S_BOARD board[1];

// 	ParseFen(FEN4, board);
// 	PrintBoard(board);
// 	ASSERT(CheckBoard(board));

// 	int move = 0;
// 	int from = 6, to = 12;
// 	int cap = wR, prom = bR;
// 	move = ((from) | (to << 7) | (cap << 14) | (prom << 20));

// 	printf("\ndec: %d\nhex: %x\n", move, move);
// 	PrintBin(move);

// 	printf("from: %d to: %d cap: %d prom: %d\n", FROMSQ(move), TOSQ(move), CAPTURED(move), PROMOTED(move));

// 	// move |= MFLAGPS;
// 	printf("is PST: %s\n", (move & MFLAGPS) ? "YES" : "NO");

// -----------------------------------------------