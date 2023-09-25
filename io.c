// io.c

#include <stdio.h>
#include "defs.h"

char *PrSq(const int sq) {
    
    static char SqStr[3];

    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a' + file), ('1' + rank));

    return SqStr;
}

char *PrMove(const int move)
{
    static char MvStr[6];

    int ff = FilesBrd[FROMSQ(move)]; // file from
    int rf = RanksBrd[FROMSQ(move)]; // rank from
 
    int ft = FilesBrd[TOSQ(move)]; // file to
    int rt = RanksBrd[TOSQ(move)]; // rank to

    int promoted = PROMOTED(move);

    if (promoted) {
        // will run if promoted
        char pchar = 'q'; // assuming promoted to queen
        if (IsKn(promoted)) // if promoted pawn to knight
        {
            pchar = 'n';
        }
        else if (IsRQ(promoted) && !IsBQ(promoted)) { // if promoted pawn to rook
            pchar = 'r';
        }
        else if (!IsRQ(promoted) && IsBQ(promoted)) { // if promoted pawn to bishop
            pchar = 'b';
        }
        sprintf(MvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar); 
    }
    else {
        // will run if not promoted
        sprintf(MvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
    }

    return MvStr;
}