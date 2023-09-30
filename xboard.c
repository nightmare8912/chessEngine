// xboard.c

#include <stdio.h>
#include <string.h> 
#include "defs.h"

// new -> new game
// sd 8 -> depth 8
// usermove e2e4 -> user has made this move now engine's turn
// move e7e5 -> our move 

int ThreeFoldRep(const S_BOARD *pos) {
    int i = 0, r = 0;
    for (i = 0; i < pos->histPly; ++i) {
        if (pos->history[i].posKey == pos->posKey) {
            r++;
        }
    }
    return r;
}
// position is a draw if neither side can give mate
int DrawMaterial(const S_BOARD *pos) {
    if (pos->pceNum[wP] || pos->pceNum[bP]) return FALSE;
    if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR]) return FALSE;
    if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1) return FALSE;
    if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1) return FALSE;
    if (pos->pceNum[wN] || pos->pceNum[wB]) return FALSE;
    if (pos->pceNum[bN] || pos->pceNum[bB]) return FALSE;

    return TRUE; 
}