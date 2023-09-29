// search.c

#include <stdio.h>
#include "defs.h"

static void CheckUp() {
	// called after approx 4000 nodes, to check if time up or interrupt from GUI
}

static int IsRepitition(const S_BOARD *pos) {
	int index = 0;

	for (index = pos->histPly - pos->fiftyMove/*because if capture is made, or pawn is moved, then no need to check for repitition*/; index < pos->histPly - 1; ++index) {
		ASSERT(index >= 0 && index <= MAXGAMEMOVES);
		if (pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}
	return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {
	// clears various stats and history 
	int index = 0;
	int index2 = 0;

	for (index = 0; index < 13; ++index) {
		for (index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for (index = 0; index < 2; ++index) {
		for (index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	ClearPvTable(pos->PvTable);
	pos->ply = 0;

	info->startTime = GetTimeMs();
	info->stopped = 0;
	info->nodes = 0;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull) {
	return 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {
	return 0;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info) {
	// will manage iterative deepening
	// for depth = 1 to maxdepth
		// search with alpha beta
}