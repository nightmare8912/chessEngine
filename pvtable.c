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
//     int index = pos->posKey % pos->HashTable->numEntries;
	
// 	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
//     ASSERT(depth>=1&&depth<MAXDEPTH);
//     ASSERT(alpha<beta);
//     ASSERT(alpha>=-INFINITE&&alpha<=INFINITE);
//     ASSERT(beta>=-INFINITE&&beta<=INFINITE);
//     ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
// 	if( pos->HashTable->pTable[index].posKey == pos->posKey ) {
// 		*move = pos->HashTable->pTable[index].move;
// 		if(pos->HashTable->pTable[index].depth >= depth){
// 			pos->HashTable->hit++;
			
// 			ASSERT(pos->HashTable->pTable[index].depth>=1&&pos->HashTable->pTable[index].depth<MAXDEPTH);
//             ASSERT(pos->HashTable->pTable[index].flags>=HFALPHA&&pos->HashTable->pTable[index].flags<=HFEXACT);
			
// 			*score = pos->HashTable->pTable[index].score;
// 			if(*score > ISMATE) *score -= pos->ply;
//             else if(*score < -ISMATE) *score += pos->ply;
			
// 			switch(pos->HashTable->pTable[index].flags) {
				
//                 ASSERT(*score>=-INFINITE&&*score<=INFINITE);

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
//     int index = pos->posKey % pos->HashTable->numEntries;
	
// 	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
// 	ASSERT(depth>=1&&depth<MAXDEPTH);
//     ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
//     ASSERT(score>=-INFINITE&&score<=INFINITE);
//     ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
// 	if( pos->HashTable->pTable[index].posKey == 0) {
// 		pos->HashTable->newWrite++;
// 	} else {
// 		pos->HashTable->overWrite++;
// 	}
	
// 	if(score > ISMATE) score += pos->ply;
//     else if(score < -ISMATE) score -= pos->ply;
	
// 	pos->HashTable->pTable[index].move = move;
//     pos->HashTable->pTable[index].posKey = pos->posKey;
// 	pos->HashTable->pTable[index].flags = flags;
// 	pos->HashTable->pTable[index].score = score;
// 	pos->HashTable->pTable[index].depth = depth;
// }

// pvtable.c

#include "stdio.h"
#include "defs.h"

#define MB 0x100000



void ClearHashTable(S_HASHTABLE *table) {

  S_HASHENTRY *tableEntry;
  
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    tableEntry->posKey = 0ULL;
    tableEntry->move = NOMOVE;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;
  }
  table->newWrite=0;
}

void InitHashTable(S_HASHTABLE *table, int sizeInMb) {  
	
	int HashSize = 0x100000 * sizeInMb;
    table->numEntries = HashSize / sizeof(S_HASHENTRY);
    table->numEntries -= 2;
	
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

int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {

	int index = pos->posKey % pos->HashTable->numEntries;
	
	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
    ASSERT(depth>=1&&depth<MAXDEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-INFINITE&&alpha<=INFINITE);
    ASSERT(beta>=-INFINITE&&beta<=INFINITE);
    ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
	if( pos->HashTable->pTable[index].posKey == pos->posKey ) {
		*move = pos->HashTable->pTable[index].move;
		if(pos->HashTable->pTable[index].depth >= depth){
			pos->HashTable->hit++;
			
			ASSERT(pos->HashTable->pTable[index].depth>=1&&pos->HashTable->pTable[index].depth<MAXDEPTH);
            ASSERT(pos->HashTable->pTable[index].flags>=HFALPHA&&pos->HashTable->pTable[index].flags<=HFEXACT);
			
			*score = pos->HashTable->pTable[index].score;
			if(*score > ISMATE) *score -= pos->ply;
            else if(*score < -ISMATE) *score += pos->ply;
			
			switch(pos->HashTable->pTable[index].flags) {
				
                ASSERT(*score>=-INFINITE&&*score<=INFINITE);

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
void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {

	int index = pos->posKey % pos->HashTable->numEntries;
	
	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
	ASSERT(depth>=1&&depth<MAXDEPTH);
    ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
    ASSERT(score>=-INFINITE&&score<=INFINITE);
    ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
	if( pos->HashTable->pTable[index].posKey == 0) {
		pos->HashTable->newWrite++;
	} else {
		pos->HashTable->overWrite++;
	}
	
	if(score > ISMATE) score += pos->ply;
    else if(score < -ISMATE) score -= pos->ply;
	
	pos->HashTable->pTable[index].move = move;
    pos->HashTable->pTable[index].posKey = pos->posKey;
	pos->HashTable->pTable[index].flags = flags;
	pos->HashTable->pTable[index].score = score;
	pos->HashTable->pTable[index].depth = depth;
}

int ProbePvMove(const S_BOARD *pos) {

	int index = pos->posKey % pos->HashTable->numEntries;
	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
	
	if( pos->HashTable->pTable[index].posKey == pos->posKey ) {
		return pos->HashTable->pTable[index].move;
	}
	
	return NOMOVE;
}


int GetPvLine(const int depth, S_BOARD *pos) {

	ASSERT(depth < MAXDEPTH && depth >= 1);

	int move = ProbePvMove(pos);
	int count = 0;
	
	while(move != NOMOVE && count < depth) {
	
		ASSERT(count < MAXDEPTH);
	
		if( MoveExists(pos, move) ) {
			MakeMove(pos, move);
			pos->PvArray[count++] = move;
		} else {
			break;
		}		
		move = ProbePvMove(pos);	
	}
	
	while(pos->ply > 0) {
		TakeMove(pos);
	}
	
	return count;
	
}

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

void StorePvMove(const S_BOARD *pos, const int move) {

    int index = pos->posKey % pos->PvTable->numEntries;
    ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
    pos->PvTable->pTable[index].move = move;
    pos->PvTable->pTable[index].posKey = pos->posKey;
    ProbePvMove(pos);
}