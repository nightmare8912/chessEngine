#include <stdio.h>
#include "defs.h"
#include "sample_fens.h"

int main()
{
	AllInit();

	S_BOARD board[1];
	S_MOVELIST list[1];

	ParseFen(START_FEN, board);

	// PerftTest(5, board);

	char input[6];
	int Move = NOMOVE;
	int PvNum = 0;
	int Max = 0;
	while(TRUE) {
		PrintBoard(board);
		printf("Please enter a move > ");
		fgets(input, 6, stdin);

		if (input[0] == 'q') {
			break;
		} 
		else if (input[0] == 't') {
			TakeMove(board);
		}
		else if (input[0] == 'p') {
			PerftTest(4, board);
		}
		else if (input[0] == 'r') {
			Max = GetPvLine(4, board);
			printf("Pvline of %d moves: ", Max);
			for (PvNum = 0; PvNum < Max; ++PvNum) {
				Move = board->PvArray[PvNum];
				printf(" %s", PrMove(Move));
			}
		}
		else {
			Move = ParseMove(input, board);
			if (Move != NOMOVE) {
				StorePvMove(board, Move);  
				MakeMove(board, Move); 
				// if (IsRepitition(board)) {
				// 	printf("repitition detected\n\n");
				// }
			}
			else {
				printf("\nInvalid move!\n");
			}
		}

		fflush(stdin);
	}

    return 0;
}
