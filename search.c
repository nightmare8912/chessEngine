// search.c

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "tinycthread.h"

int rootDepth;
thrd_t workerThreads[MAXTHREADS];

void SearchPositionMultiThreading(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);

static void CheckUp(S_SEARCHINFO *info)
{
	// called after approx 4000 nodes, to check if time up or interrupt from GUI
	if (info->timeSet == TRUE && GetTimeMs() > info->stopTime)
	{
		info->stopped = TRUE;
	}
	// ReadInput(info);
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

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table)
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

	pos->ply = 0;

	table->overWrite = 0;
	table->hit = 0;
	table->cut = 0;
	pos->ply = 0;
	table->currentAge++;
	// info->startTime = GetTimeMs();
	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info)
{
	ASSERT(CheckBoard(pos));

	if ((info->nodes & 2047) == 0)
	{
		// checking if time limit reached at every 2047 nodes
		CheckUp(info);
	}

	info->nodes++;

	if (IsRepitition(pos) || pos->fiftyMove >= 100)
	{
		return 0; // draw
	}
	if (pos->ply > MAXDEPTH - 1)
	{
		return EvalPosition(pos);
	}
	int Score = EvalPosition(pos);

	if (Score >= beta)
	{
		return beta; // no changes made as alpha beta as found best move already
	}

	if (Score > alpha)
	{
		alpha = Score;
	}

	S_MOVELIST list[1];
	GenerateAllCaps(pos, list);

	int MoveNum = 0;
	int Legal = 0; // number of legal moves possible, in checkmate or stalemate it is zero
	// int OldAlpha = alpha;
	// int BestMove = NOMOVE;
	Score = -AB_BOUND;

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

		if (info->stopped == TRUE)
		{
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
			// BestMove = list->moves[MoveNum].move;
		}
	}

	// if (alpha != OldAlpha) {
	// 	StorePvMove(pos, BestMove);
	// }
	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table, int DoNull)
{

	ASSERT(CheckBoard(pos));

	if (depth <= 0)
	{
		// info->nodes++;
		return Quiescence(alpha, beta, pos, info);
	}

	if ((info->nodes & 2047) == 0)
	{
		// checking if time limit reached at every 2047 nodes
		CheckUp(info);
	}

	info->nodes++;

	if ((IsRepitition(pos) || pos->fiftyMove >= 100) && pos->ply)
	{
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1)
	{
		return EvalPosition(pos);
	}

	int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);

	if (InCheck == TRUE)
	{
		depth++;
	}

	int Score = -AB_BOUND;
	int PvMove = NOMOVE;

	if (ProbeHashEntry(pos, table, &PvMove, &Score, alpha, beta, depth) == TRUE)
	{
		table->cut++;
		return Score;
	}

	if (DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 1) && depth >= 4)
	{
		MakeNullMove(pos);
		Score = -AlphaBeta(-beta, -beta + 1, depth - 4, pos, info, table, FALSE);
		TakeNullMove(pos);
		if (info->stopped == TRUE)
		{
			return 0;
		}
		if (Score >= beta && abs(Score) < ISMATE)
		{
			return beta;
		}
	}

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	int MoveNum = 0;
	int Legal = 0; // number of legal moves possible, in checkmate or stalemate it is zero
	int OldAlpha = alpha;
	int BestMove = NOMOVE;
	int BestScore = -AB_BOUND;

	Score = -AB_BOUND;

	if (PvMove != NOMOVE)
	{
		for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
		{
			if (list->moves[MoveNum].move == PvMove)
			{
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
		Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, table, TRUE);
		TakeMove(pos);

		if (info->stopped == TRUE)
		{
			return 0;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
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

					if (!(list->moves[MoveNum].move & MFLAGCAP))
					{
						// not a capture (beta cutoff area)
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}
					StoreHashEntry(pos, table, BestMove, beta, HFBETA, depth);
					return beta;
				}
				alpha = Score;
				BestMove = list->moves[MoveNum].move;
				if (!(list->moves[MoveNum].move & MFLAGCAP))
				{
					// not a capture (alpha cutoff area | improve search history)
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
					// incremented by depth so that the moves closer to root of tree are prioritized
				}
			}
		}
	}

	if (Legal == 0)
	{
		if (InCheck)
		{
			// check mate
			return -AB_BOUND + pos->ply; // mate in 5 or 6, can be used to extract in how many steps are we getting mated
		}
		else
		{
			// stale mate
			return 0;
		}
	}

	if (alpha != OldAlpha)
	{
		// StorePvMove(pos, BestMove);
		StoreHashEntry(pos, table, BestMove, BestScore, HFEXACT, depth);
	}
	else
	{
		StoreHashEntry(pos, table, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}


int SearchPositionThread(void *data) {
	S_SEARCH_THREAD_DATA *searchData = (S_SEARCH_THREAD_DATA *) data;
	// S_BOARD *pos = malloc(sizeof(S_BOARD));
	S_BOARD *pos = searchData->originalPosition;
	// memcpy(pos, searchData->originalPosition, sizeof(S_BOARD));
	// we need a complete new copy of the board as each of the thread will be manipulating the board differently
	// whereas info and HashTable will be the same for all of them
	SearchPositionMultiThreading(pos, searchData->info, searchData->ttable);
	// free(pos);
	// printf("Freed!\n");
	return 0;
}

void IterativeDeepen(S_SEARCH_WORKER_DATA *workerData) {
	
	workerData->bestMove = NOMOVE;
	int bestScore = -AB_BOUND;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;


	for (currentDepth = 1; currentDepth <= workerData->info->depth; ++currentDepth)
	{
		rootDepth = currentDepth;
		bestScore = AlphaBeta(-AB_BOUND, AB_BOUND, currentDepth, workerData->pos, workerData->info, workerData->ttable, TRUE);

		if (workerData->info->stopped == TRUE)
		{
			break;
		}

		if (workerData->threadNumber == 0) {
			pvMoves = GetPvLine(currentDepth, workerData->pos, workerData->ttable);
			workerData->bestMove = workerData->pos->PvArray[0];

			if (workerData->info->GAME_MODE == UCIMODE)
			{
				printf("info score cp %d depth %d nodes %ld time %d ",
					   bestScore, currentDepth, workerData->info->nodes, GetTimeMs() - workerData->info->startTime);
			}
			else if (workerData->info->GAME_MODE == XBOARDMODE && workerData->info->POST_THINKING == TRUE)
			{
				printf("%d %d %d %ld ",
					   currentDepth, bestScore, (GetTimeMs() - workerData->info->startTime) / 10, workerData->info->nodes);
			}
			else if (workerData->info->POST_THINKING == TRUE)
			{
				printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
					   bestScore, currentDepth, workerData->info->nodes, GetTimeMs() - workerData->info->startTime);
			}
			if (workerData->info->GAME_MODE == UCIMODE || workerData->info->POST_THINKING == TRUE)
			{
				if (!workerData->info->GAME_MODE == XBOARDMODE)
				{
					printf("pv");
				}
				for (pvNum = 0; pvNum < pvMoves; ++pvNum)
				{
					printf(" %s", PrMove(workerData->pos->PvArray[pvNum]));
				}
				printf("\n");
			}
		}		
	}
}

int StartWorkerThread(void *data) {
	S_SEARCH_WORKER_DATA *workerData = (S_SEARCH_WORKER_DATA *)data;
	// printf("Thread %d starts\n", workerData->threadNumber);
	IterativeDeepen(workerData);
	// printf("Thread %d ends\n", workerData->threadNumber);
		if (workerData->threadNumber == 0) {/*last thread ends*/
			if (workerData->info->GAME_MODE == UCIMODE)
			{
				printf("bestmove %s\n", PrMove(workerData->bestMove));
			}
			else if (workerData->info->GAME_MODE == XBOARDMODE)
			{
				printf("move %s\n", PrMove(workerData->bestMove));
				MakeMove(workerData->globalPos, workerData->bestMove);
			}
			else
			{
				printf("\n\n***!!Vince makes move %s using search!!***\n\n", PrMove(workerData->bestMove));
				MakeMove(workerData->globalPos, workerData->bestMove);
				PrintBoard(workerData->globalPos);
			}
	}
	free(workerData);
}

void SetUpWorker(int threadNum, thrd_t *workerTh, S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {
	S_SEARCH_WORKER_DATA *pWorkerData = (S_SEARCH_WORKER_DATA *) malloc (sizeof(S_SEARCH_WORKER_DATA));
	pWorkerData->pos = malloc(sizeof(S_BOARD));
	memcpy(pWorkerData->pos, pos, sizeof(S_BOARD));
	pWorkerData->info = info;
	pWorkerData->ttable = table;
	pWorkerData->globalPos = pos;
	pWorkerData->threadNumber = threadNum;
	thrd_create(workerTh, &StartWorkerThread, (void *)pWorkerData);
}

void CreateSearchWorkers(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {
	
	// printf("Create search workers: %d\n", info->threadNum);
	for (int i = 0; i < info->threadNum; ++i) {
		SetUpWorker(i, &workerThreads[i], pos, info, table);
	}
}

void SearchPositionMultiThreading(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table)
{
	int bestMove = NOMOVE;
	ClearForSearch(pos, info, table);

	if (EngineOptions->UseBook == TRUE) {
		bestMove = GetBookMove(pos);
		if (bestMove != NOMOVE) {
			if (info->GAME_MODE == UCIMODE)
			{
				printf("bestmove %s\n", PrMove(bestMove));
			}
			else if (info->GAME_MODE == XBOARDMODE)
			{
				printf("move %s\n", PrMove(bestMove));
				MakeMove(pos, bestMove);
			}
			else
			{
				printf("\n\n***!!Vince makes move %s using book!!***\n\n", PrMove(bestMove));
				MakeMove(pos, bestMove);
				PrintBoard(pos);
			}
			return;
		}
	}

	if (bestMove == NOMOVE) {
		CreateSearchWorkers(pos, info, table);
	}
	for (int i = 0; i < info->threadNum; i++) {
		thrd_join(workerThreads[i], NULL);
	}
}



void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table)
{
	// will manage iterative deepening
	// for depth = 1 to maxdepth
	// search with alpha beta
	// goto next depth

	int bestMove = NOMOVE;
	int bestScore = -AB_BOUND;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos, info, table);

	if (EngineOptions->UseBook == TRUE) {
		bestMove = GetBookMove(pos);
	}

	if (bestMove == NOMOVE) {

		for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth)
		{
			bestScore = AlphaBeta(-AB_BOUND, AB_BOUND, currentDepth, pos, info, table, TRUE);

			if (info->stopped == TRUE)
			{
				break;
			}

			pvMoves = GetPvLine(currentDepth, pos, table);
			bestMove = pos->PvArray[0];

			// pvMoves = GetPvLine(currentDepth, pos);
			if (info->GAME_MODE == UCIMODE)
			{
				printf("info score cp %d depth %d nodes %ld time %d ", bestScore, currentDepth, info->nodes, GetTimeMs() - info->startTime);
			}

			else if (info->GAME_MODE == XBOARDMODE && info->POST_THINKING == TRUE)
			{
				printf("%d %d %d %ld ", currentDepth, bestScore, (GetTimeMs() - info->startTime) / 10, info->nodes);
			}

			else if (info->POST_THINKING == TRUE)
			{
				printf("score cp %d depth %d nodes %ld time %d ", bestScore, currentDepth, info->nodes, GetTimeMs() - info->startTime);
			}

			if (info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE)
			{
				// pvMoves = GetPvLine(currentDepth, pos, table);
				printf("pv");
				for (pvNum = 0; pvNum < pvMoves; ++pvNum)
				{
					printf(" %s", PrMove(pos->PvArray[pvNum]));
				}
				printf("\n");
				printf("Ordering: %.2f\n", (info->fhf / info->fh));
			}
		}
	}
	
	if (info->GAME_MODE == UCIMODE)
	{
		printf("bestmove %s\n", PrMove(bestMove));
	}
	else if (info->GAME_MODE == XBOARDMODE)
	{
		printf("move %s\n", PrMove(bestMove));
		MakeMove(pos, bestMove);
	}
	else
	{
		printf("\n\n***!!Vince makes move %s !!***\n\n", PrMove(bestMove));
		MakeMove(pos, bestMove);
		PrintBoard(pos);
	}
}

