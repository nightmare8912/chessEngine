// pvtable.c

#include "stdio.h"
#include "defs.h"

#define MB 0x100000

#define EXTRACT_SCORE(x) ((x & 0xFFFF) - INF_BOUND)
#define EXTRACT_DEPTH(x) ((x >> 16) & 0x3F)
#define EXTRACT_FLAGS(x) ((x >> 23) & 0x3)
#define EXTRACT_MOVE(x) ((int)(x >> 25))

#define FOLD_DATA(sc, de, fl, mv) ((sc + INF_BOUND) | (de << 16) | (fl << 23) | ((U64)mv << 25))

void DataCheck(const int move) {
	int depth = rand() % MAXDEPTH;
	int flag = rand() % 3;
	int score = rand() % AB_BOUND;

	U64 data = FOLD_DATA(score, depth, flag, move);

	printf("Orig Move: %s, depth: %d, flag: %d, score: %d, data: %llX\n", 
			PrMove(move), depth, flag, score, data);
	printf("Extract Move: %s, depth: %d, flag: %d, score: %d\n\n", 
			PrMove(EXTRACT_MOVE(data)), 
			EXTRACT_DEPTH(data),
			EXTRACT_FLAGS(data), 
			EXTRACT_SCORE(data));
}

void TempHashTest(char *fen) {
	S_BOARD board[1];
	ParseFen(fen, board);

	S_MOVELIST list[1];
    GenerateAllMoves(board, list);

    int move;
    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
    {
        move = list->moves[MoveNum].move;
        if (!MakeMove(board, move))
        {
            continue;
        }
        TakeMove(board);
        DataCheck(move);
    }
}

void VerifyEntrySMP(S_HASHENTRY *entry) {
	/*
	U64 data = FOLD_DATA(entry->score, entry->depth, entry->flags, entry->move);
	U64 key = entry->posKey ^ data;

	if (data != entry->smp_data) {
		printf("data error\n");
		exit(1);
	}
	if (key != entry->smp_key) {
		printf("key error\n");
		exit(1);
	}

	int move = EXTRACT_MOVE(data);
	int flag = EXTRACT_FLAGS(data);
	int score = EXTRACT_SCORE(data);
	int depth = EXTRACT_DEPTH(data);

	if (move != entry->move) {
		printf("Move error!\n");
	}
	if (score != entry->score) {
		printf("score error!\n");
	}
	if (flag != entry->flags) {
		printf("flag error!\n");
	}
	if (depth != entry->depth) {
		printf("depth error!\n");
	}
	*/
}

S_HASHTABLE HashTable[1];

void ClearHashTable(S_HASHTABLE *table) {

  S_HASHENTRY *tableEntry;
  
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    /*
	tableEntry->posKey = 0ULL;
    tableEntry->move = NOMOVE;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;
	*/
	tableEntry->age = 0;
	tableEntry->smp_data = 0ULL;
	tableEntry->smp_key = 0ULL;
  }
  table->newWrite = 0;
  table->currentAge = 0;
}

void InitHashTable(S_HASHTABLE *table, int sizeInMb) {  
	
	int HashSize = 0x100000 * sizeInMb;
    table->numEntries = HashSize / sizeof(S_HASHENTRY);
    table->numEntries -= 2;
    table->numEntries = 1000000;
	
	if(table->pTable!=NULL) {
		free(table->pTable);
	}
		
    table->pTable = (S_HASHENTRY *) malloc(table->numEntries * sizeof(S_HASHENTRY));
	// printf("Size of table: %d\n", sizeof(table->pTable));
	if(table->pTable == NULL) {
		printf("Hash Allocation Failed, trying %dMB...\n",sizeInMb / 2);
		InitHashTable(table,sizeInMb/2);
	} else {
		ClearHashTable(table);
		printf("HashTable init complete with %d entries\n",table->numEntries);
	}
	
}

int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int alpha, int beta, int depth) {

	int index = pos->posKey % table->numEntries;
	
	ASSERT(index >= 0 && index <= table->numEntries - 1);
    ASSERT(depth>=1&&depth<MAXDEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-AB_BOUND&&alpha<=AB_BOUND);
    ASSERT(beta>=-AB_BOUND&&beta<=AB_BOUND);
    ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	U64 test_key = pos->posKey ^ table->pTable[index].smp_data;

	if(table->pTable[index].smp_key == test_key) {
		
		int smp_depth = EXTRACT_DEPTH(table->pTable[index].smp_data);
		int smp_move = EXTRACT_MOVE(table->pTable[index].smp_data);
		int smp_score = EXTRACT_SCORE(table->pTable[index].smp_data);
		int smp_flags = EXTRACT_FLAGS(table->pTable[index].smp_data);

		*move = smp_move;
		if(smp_depth >= depth){
			
			table->hit++;
			
			*score = smp_score;
			if(*score > ISMATE) *score -= pos->ply;
            else if(*score < -ISMATE) *score += pos->ply;
			
			switch(smp_flags) {
				
                ASSERT(*score>=-AB_BOUND&&*score<=AB_BOUND);

                case HFALPHA: if(*score<=alpha) {
                    *score=alpha;
                    return TRUE;
                    }
                    break;
                case HFBETA: if(*score>=beta) {
                    *score=beta;
                    return TRUE;
                    }
                    break;
                case HFEXACT:
                    return TRUE;
                    break;
                default: ASSERT(FALSE); break;
            }
		}
	}
	
	return FALSE;
}
void StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth) {

	int index = pos->posKey % table->numEntries;
	
	ASSERT(index >= 0 && index <= table->numEntries - 1);
	ASSERT(depth>=1&&depth<MAXDEPTH);
    ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
    ASSERT(score>=-AB_BOUND&&score<=AB_BOUND);
    ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
	int replace = FALSE;

	if( table->pTable[index].smp_key == 0) {
		table->newWrite++;
		replace = TRUE; // the index is empty
	} else {
		table->overWrite++;
		// if (table->pTable[index].age < table->currentAge || table->pTable[index].depth <= depth) {
		// 	replace = TRUE; // we're replacing already existing value
		// }

		if (table->pTable[index].age < table->currentAge) {
			replace = TRUE;
		}
		else if (EXTRACT_DEPTH(table->pTable[index].smp_data) <= depth) {
			replace = TRUE;
		}
	}

	if (replace == FALSE) return;
	
	if(score > ISMATE) score += pos->ply;
    else if(score < -ISMATE) score -= pos->ply;

	U64 smp_data = FOLD_DATA(score, depth, flags, move);
	
	/*
	U64 smp_key = pos->posKey ^ smp_data;
	
	table->pTable[index].move = move;
    table->pTable[index].posKey = pos->posKey;
	table->pTable[index].flags = flags;
	table->pTable[index].score = score;
	table->pTable[index].depth = depth;
	*/
	table->pTable[index].smp_data = smp_data;
	table->pTable[index].smp_key = pos->posKey ^ smp_data;
	table->pTable[index].age = table->currentAge;

	VerifyEntrySMP(&table->pTable[index]);
}

int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE *table) {

	int index = pos->posKey % table->numEntries;
	U64 test_key = pos->posKey ^ table->pTable[index].smp_data;
	ASSERT(index >= 0 && index <= table->numEntries - 1);
	
	if(table->pTable[index].smp_key == test_key) {
		return EXTRACT_MOVE(table->pTable[index].smp_data);
	}
	
	return NOMOVE;
}


int GetPvLine(const int depth, S_BOARD *pos, const S_HASHTABLE *table) {

	ASSERT(depth < MAXDEPTH && depth >= 1);

	int move = ProbePvMove(pos, table);
	int count = 0;
	
	while(move != NOMOVE && count < depth) {
	
		ASSERT(count < MAXDEPTH);
	
		if( MoveExists(pos, move) ) {
			MakeMove(pos, move);
			pos->PvArray[count++] = move;
		} else {
			break;
		}		
		move = ProbePvMove(pos, table);	
	}
	
	while(pos->ply > 0) {
		TakeMove(pos);
	}
	
	return count;
	
}



const int PvSize = MB * 2; // 2mb in size
const int HashSize = MB * 16; // 16mb in size

void ClearPvTable(S_PVTABLE *table) {
    
    S_PVENTRY *pvEntry;
    for (pvEntry = table->pTable; pvEntry < table->pTable + table->numEntries; pvEntry++) {
        pvEntry->posKey = 0ULL;
        pvEntry->move = NOMOVE;
    }
}

void InitPvTable(S_PVTABLE *table) {
    
    table->numEntries = PvSize / sizeof(S_PVENTRY);
    table->numEntries -= 2; // making sure that while accessing the table we don't go out of index (extra security)
    if (table->pTable != NULL) {
        free(table->pTable); // free if any memory is pointed
    }
    table->pTable = (S_PVENTRY *) malloc (table->numEntries * sizeof(S_PVENTRY));
    ClearPvTable(table);
    printf("PvTableInit completed with %d entries\n", table->numEntries);
}

// void StorePvMove(const S_BOARD *pos, const int move) {

//     int index = pos->posKey % pos->PvTable->numEntries;
//     ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
//     pos->PvTable->pTable[index].move = move;
//     pos->PvTable->pTable[index].posKey = pos->posKey;
//     ProbePvMove(pos, table);
// }


//------------------------------------------
// this function is used to load the Principle Variation table for a given depth into the PVArray
// int GetPvLine(const int depth, S_BOARD *pos) {
    
//     ASSERT(depth < MAXDEPTH);
//     int move = ProbePvMove(pos);
//     int count = 0;
//     PrMove(move);
//     while (move != NOMOVE && count < depth) {
        
//         ASSERT(count < MAXDEPTH);

//         if (MoveExists(pos, move)) {
//             MakeMove(pos, move);
//             pos->PvArray[count] = move;
//             count++;
//             // we're making a move and checking for next move
//         }
//         else {
//             printf("Breaken out!\n");
//             break;
//         }
//         move = ProbePvMove(pos);
//     }
//     // undoing the move
//     while (pos->ply > 0) {
//         TakeMove(pos);
//     }
//     return count;
// }



// // pvtable.c

// #include <stdio.h>
// #include "defs.h"

// #define MB 0x100000


// void ClearHashTable(S_HASHTABLE *table) {
    
//     S_HASHENTRY *tableEntry;
//     for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
//         tableEntry->posKey = 0ULL;
//         tableEntry->move = NOMOVE;
//         tableEntry->depth = 0;
//         tableEntry->score = 0;
//         tableEntry->flags = 0;
//     }
//     table->newWrite = 0;
// }

// void InitHashTable(S_HASHTABLE *table) {
//     table->numEntries = HashSize / sizeof(S_PVENTRY);
//     table->numEntries = HashSize / sizeof(S_HASHENTRY);
//     table->numEntries -= 2;
	
// 	if(table->pTable!=NULL) {
// 		free(table->pTable);
// 	}
		
//     table->pTable = (S_HASHENTRY *) malloc(table->numEntries * sizeof(S_HASHENTRY));
// 	// printf("Size of table: %d\n", sizeof(table->pTable));
	
//     ClearHashTable(table);
//     printf("HashTable init complete with %d entries\n",table->numEntries);
	
// }

// int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {
//     int index = pos->posKey % table->numEntries;
	
// 	ASSERT(index >= 0 && index <= table->numEntries - 1);
//     ASSERT(depth>=1&&depth<MAXDEPTH);
//     ASSERT(alpha<beta);
//     ASSERT(alpha>=-AB_BOUND&&alpha<=AB_BOUND);
//     ASSERT(beta>=-AB_BOUND&&beta<=AB_BOUND);
//     ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
// 	if( table->pTable[index].posKey == pos->posKey ) {
// 		*move = table->pTable[index].move;
// 		if(table->pTable[index].depth >= depth){
// 			table->hit++;
			
// 			ASSERT(table->pTable[index].depth>=1&&table->pTable[index].depth<MAXDEPTH);
//             ASSERT(table->pTable[index].flags>=HFALPHA&&table->pTable[index].flags<=HFEXACT);
			
// 			*score = table->pTable[index].score;
// 			if(*score > ISMATE) *score -= pos->ply;
//             else if(*score < -ISMATE) *score += pos->ply;
			
// 			switch(table->pTable[index].flags) {
				
//                 ASSERT(*score>=-AB_BOUND&&*score<=AB_BOUND);

//                 case HFALPHA: if(*score<=alpha) {
//                     *score=alpha;
//                     return TRUE;
//                     }
//                     break;
//                 case HFBETA: if(*score>=beta) {
//                     *score=beta;
//                     return TRUE;
//                     }
//                     break;
//                 case HFEXACT:
//                     return TRUE;
//                     break;
//                 default: ASSERT(FALSE); break;
//             }
// 		}
// 	}
	
// 	return FALSE;
// }

// void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {
//     int index = pos->posKey % table->numEntries;
	
// 	ASSERT(index >= 0 && index <= table->numEntries - 1);
// 	ASSERT(depth>=1&&depth<MAXDEPTH);
//     ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
//     ASSERT(score>=-AB_BOUND&&score<=AB_BOUND);
//     ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
// 	if( table->pTable[index].posKey == 0) {
// 		table->newWrite++;
// 	} else {
// 		table->overWrite++;
// 	}
	
// 	if(score > ISMATE) score += pos->ply;
//     else if(score < -ISMATE) score -= pos->ply;
	
// 	table->pTable[index].move = move;
//     table->pTable[index].posKey = pos->posKey;
// 	table->pTable[index].flags = flags;
// 	table->pTable[index].score = score;
// 	table->pTable[index].depth = depth;
// }
