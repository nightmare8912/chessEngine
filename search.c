// search.c

#include <stdio.h>
#include "defs.h"

#define INFINITE 30000
#define MATE 29000

static void CheckUp(S_SEARCHINFO *info)
{
	// called after approx 4000 nodes, to check if time up or interrupt from GUI
	if (info->timeSet == TRUE && GetTimeMs() > info->stopTime) {
		info->stopped = TRUE;
	}
	ReadInput(info);
}

static void PickNextMove(int moveNum, S_MOVELIST *list)
{

	// the reason for calling this function is because our alpha beta will become more efficient
	// as this puts the best moves in earlier index!

	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index)
	{
		if (list->moves[index].score > bestScore)
		{
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}
	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepitition(const S_BOARD *pos)
{
	int index = 0;

	for (index = pos->histPly - pos->fiftyMove /*because if capture is made, or pawn is moved, then no need to check for repitition*/; index < pos->histPly - 1; ++index)
	{
		ASSERT(index >= 0 && index <= MAXGAMEMOVES);
		if (pos->posKey == pos->history[index].posKey)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info)
{
	// clears various stats and history
	int index = 0;
	int index2 = 0;

	for (index = 0; index < 13; ++index)
	{
		for (index2 = 0; index2 < BRD_SQ_NUM; ++index2)
		{
			pos->searchHistory[index][index2] = 0;
		}
	}

	for (index = 0; index < 2; ++index)
	{
		for (index2 = 0; index2 < MAXDEPTH; ++index2)
		{
			pos->searchKillers[index][index2] = 0;
		}
	}

	ClearPvTable(pos->PvTable);
	pos->ply = 0;

	info->startTime = GetTimeMs();
	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info)
{
	ASSERT(CheckBoard(pos));

	if ((info->nodes & 2047) == 0) {
		// checking if time limit reached at every 2047 nodes
		CheckUp(info);
	}

	info->nodes++;

	if (IsRepitition(pos) || pos->fiftyMove >= 100) {
		return 0; // draw
	}
	if (pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}
	int Score = EvalPosition(pos);

	if (Score >= beta) {
		return beta; // no changes made as alpha beta as found best move already
	}

	if (Score > alpha) {
		alpha = Score;
	}

	S_MOVELIST list[1];
	GenerateAllCaps(pos, list);

	int MoveNum = 0;
	int Legal = 0; // number of legal moves possible, in checkmate or stalemate it is zero
	int OldAlpha = alpha;
	int BestMove = NOMOVE;
	Score = -INFINITE;
	int PvMove = ProbePvTable(pos);

	for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
	{

		PickNextMove(MoveNum, list);

		if (!MakeMove(pos, list->moves[MoveNum].move))
		{
			continue;
		}
		Legal++;
		Score = -Quiescence(-beta, -alpha, pos, info);
		TakeMove(pos);

		if (info->stopped == TRUE) {
			return 0;
		} 

		if (Score > alpha)
		{
			// alpha will be updated
			if (Score >= beta)
			{
				if (Legal == 1)
				{
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = Score;
			BestMove = list->moves[MoveNum].move;
		}
	}

	if (alpha != OldAlpha) {
		StorePvMove(pos, BestMove);
	}
	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull)
{

	ASSERT(CheckBoard(pos));

	if (depth == 0)
	{
		// info->nodes++;
		return Quiescence(alpha, beta, pos, info);
	}

	if ((info->nodes & 2047) == 0) {
		// checking if time limit reached at every 2047 nodes
		CheckUp(info);
	}

	info->nodes++;

	if (IsRepitition(pos) || pos->fiftyMove >= 100)
	{
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1)
	{
		return EvalPosition(pos);
	}

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	int MoveNum = 0;
	int Legal = 0; // number of legal moves possible, in checkmate or stalemate it is zero
	int OldAlpha = alpha;
	int BestMove = NOMOVE;
	int Score = -INFINITE;
	int PvMove = ProbePvTable(pos);

	if (PvMove != NOMOVE) {
		for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if (list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				break;
			}
		}
	}

	for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
	{

		PickNextMove(MoveNum, list);

		if (!MakeMove(pos, list->moves[MoveNum].move))
		{
			continue;
		}
		Legal++;
		Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE);
		TakeMove(pos);

		if (info->stopped == TRUE) {
			return 0;
		} 

		if (Score > alpha)
		{
			// alpha will be updated
			if (Score >= beta)
			{
				if (Legal == 1)
				{
					info->fhf++;
				}
				info->fh++;

				if (!(list->moves[MoveNum].move & MFLAGCAP)) {
					// not a capture (beta cutoff area)
					pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
					pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
				}

				return beta;
			}
			alpha = Score;
			BestMove = list->moves[MoveNum].move;
			if (!(list->moves[MoveNum].move & MFLAGCAP)) {
				// not a capture (alpha cutoff area | improve search history)
				pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
				// incremented by depth so that the moves closer to root of tree are prioritized
			}
		}
	}

	if (Legal == 0)
	{
		if (SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos))
		{
			// check mate
			return -MATE + pos->ply; // mate in 5 or 6, can be used to extract in how many steps are we getting mated
		}
		else
		{
			// stale mate
			return 0;
		}
	}

	if (alpha != OldAlpha)
	{
		StorePvMove(pos, BestMove);
	}

	return alpha;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info)
{
	// will manage iterative deepening
	// for depth = 1 to maxdepth
	// search with alpha beta
	// goto next depth

	int bestMove = NOMOVE;
	int bestScore = -INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos, info);

	for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth)
	{
		bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE);

		if (info->stopped == TRUE) {
			break;
		}

		pvMoves = GetPvLine(currentDepth, pos);
		bestMove = pos->PvArray[0];

		printf("info score cp %d depth %d nodes %ld time %d ", bestScore, currentDepth, info->nodes, GetTimeMs() - info->startTime);

		pvMoves = GetPvLine(currentDepth, pos);
		printf("pv");

		for (pvNum = 0; pvNum < pvMoves; ++pvNum)
		{
			printf(" %s", PrMove(pos->PvArray[pvNum]));
		}
		printf("\n");
		printf("Ordering: %.2f\n", (info->fhf / info->fh));
	}
	printf("bestmove %s\n", PrMove(bestMove));
}