// hashkeys.c
#include "defs.h"

U64 GeneratePosKey(const S_BOARD *pos)
{
    int sq = 0;
    U64 finalKey = 0;
    int piece = EMPTY;

    // pieces
    for (sq = 0; sq < BRD_SQ_NUM; ++sq)
    {
        piece = pos->pieces[sq];
        if (piece != NO_SQ && piece != EMPTY)
        {
            // if this runs that means that this square is inside the board and is not empty
            // sp the piece located here must have int value > white pawn and less than black king
            ASSERT(piece >= wP && piece <= bK);
            // so we can take the XOR and generate unique key
            finalKey ^= PieceKeys[piece][sq];
        }
    }

    if (pos->side == WHITE)
    {
        finalKey ^= SideKey; // Sidekey means whose turn it is
    }

    if (pos->enPas != NO_SQ)
    {
        ASSERT(pos->enPas >= 0 && pos->enPas < BRD_SQ_NUM);
        finalKey ^= PieceKeys[EMPTY][pos->enPas];
    }

    ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);
    finalKey ^= CastleKeys[pos->castlePerm];

    return finalKey;
}