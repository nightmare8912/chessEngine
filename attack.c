// attack.c

#include "defs.h"

const int KnDir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10,	1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

int SqAttacked(const int sq, const int side, const S_BOARD *pos)
{
    int pce, index, t_sq, dir;

    ASSERT(SqOnBoard(sq));
    ASSERT(SideValid(side));
    ASSERT(CheckBoard(pos));

    // pawns
    if (side == WHITE)
    {
        if (pos->pieces[sq - 11] == wP  || pos->pieces[sq - 9] == wP)
        {
            return TRUE;
        }
    }
    else
    {
            if (pos->pieces[sq + 11] == bP || pos->pieces[sq + 9] == bP)
            {
                return TRUE;
            }
    }

    // knights
    for(index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KnDir[index]];
		// ASSERT(PceValidEmptyOffbrd(pce));
		if (pce != OFFBOARD && IsKn(pce) && PieceCol[pce] == side) {
			return TRUE;
		}
	}

    // rooks and queens
    for (index = 0; index < 4; ++index)
    {
        dir = RkDir[index]; // we first fetch the direction
        t_sq = sq + dir; // we then add or subtract direction to current square(its the same idea of what I implemented in my pythonChess)
        pce = pos->pieces[t_sq]; // we load the piece at that square
        while(pce != OFFBOARD) // until we are off board, we iterate through squares
        {
            if (pce != EMPTY) // if piece is not empty then, 
            {
                if (IsRQ(pce) && PieceCol[pce] == side) // if the piece if of type Rook/Queen and it is turn of attacking side
                {
                    return TRUE; // then the square is attacked
                }
                break; // if this part runs, then the square that we checked was not empty and did not contain a rook, which means that this piece will now block any attack from squares in this direction, and we don't need to iterate further
            }
            t_sq += dir; // we can now increment the square and check for adjacent squares in that direction
            pce = pos->pieces[t_sq]; // we load the piece !
        }
    }

    // bishops and queens
    for (index = 0; index < 4; ++index)
    {
        dir = BiDir[index];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        while (pce != OFFBOARD)
        {
            if (pce != EMPTY)
            {
                if (IsBQ(pce) && PieceCol[pce] == side)
                {
                    return TRUE;
                }
                break;
            }
            t_sq += dir;
            pce = pos->pieces[t_sq];
        }
    }

    // kings
    for (index = 0; index < 8; ++index)
    {
        pce = pos->pieces[sq + KiDir[index]];
        if (pce != OFFBOARD && IsKi(pce) && PieceCol[pce] == side)
        {
            return TRUE;
        }
    }

    return FALSE;
}