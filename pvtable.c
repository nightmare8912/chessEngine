// pvtable.c

#include <stdio.h>
#include "defs.h"

// this function is used to load the Principle Variation table for a given depth into the PVArray
int GetPvLine(const int depth, S_BOARD *pos) {
    
    ASSERT(depth < MAXDEPTH);
    int move = ProbePvTable(pos);
    int count = 0;
    PrMove(move);
    while (move != NOMOVE && count < depth) {
        
        ASSERT(count < MAXDEPTH);

        if (MoveExists(pos, move)) {
            MakeMove(pos, move);
            pos->PvArray[count] = move;
            count++;
            // we're making a move and checking for next move
        }
        else {
            printf("Breaken out!\n");
            break;
        }
        move = ProbePvTable(pos);
    }
    // undoing the move
    while (pos->ply > 0) {
        TakeMove(pos);
    }
    return count;
}

const int PvSize = 0x100000 * 2; // 2mb in size

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
    ProbePvTable(pos);
}

int ProbePvTable(const S_BOARD *pos) {

    int index = pos->posKey % pos->PvTable->numEntries;
    ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);

    if (pos->PvTable->pTable[index].posKey == pos->posKey) {
        return pos->PvTable->pTable[index].move;
    }
    return NOMOVE;
}