// polybook.c

#include "defs.h"
#include "polykeys.h"

typedef struct {
    U64 key;
    unsigned short move;
    unsigned short weight;
    unsigned int learn;
} S_POLY_BOOK_ENTRY;

long NumEntries = 0;

S_POLY_BOOK_ENTRY *entries;

void InitPolyBook() {

    EngineOptions->UseBook = FALSE;

    FILE *pFile = fopen("performance.bin", "rb");

    if (pFile == NULL) {
        printf("Book file not read!");
    }
    else {
        fseek(pFile, 0, SEEK_END);
        long position = ftell(pFile);

        if (position < sizeof(S_POLY_BOOK_ENTRY)) {
            printf("Insufficient entries found!");
            return;
        }
        NumEntries = position / sizeof(S_POLY_BOOK_ENTRY);
        printf("%ld Entries found in file\n", NumEntries);

        entries = (S_POLY_BOOK_ENTRY *) malloc (NumEntries * sizeof(S_POLY_BOOK_ENTRY));

        rewind(pFile); // going back to begining of file

        size_t returnValue;
        returnValue = fread(entries, sizeof(S_POLY_BOOK_ENTRY), NumEntries, pFile); // reading at a time the size of poly book entry and reading a total of NumEntries
        printf("fread() %ld Entries read in from file\n", returnValue);

        if (NumEntries > 0) {
            EngineOptions->UseBook = TRUE;
        }
    }
}

void ClearPolyBook() {
    if (entries != NULL) {
        free(entries);
    }
    
}

// for the decoding of en passant, the enpassant square is only set when there is a pawn
// that can be captured, otherwise it is never set!(different from how Vince work)
const int PolyKindOfPiece[13] = {
    -1, 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10
};

// for en-passant
int HasPawnForCapture(const S_BOARD *board) {
    int sqWithPawn = 0;
    int targetPce = (board->side == WHITE) ? wP : bP;
    if (board->enPas != NO_SQ) {
        if (board->side == WHITE) {
            sqWithPawn = board->enPas - 10;
        }
        else {
            sqWithPawn = board->enPas + 10;
        }

        if (board->pieces[sqWithPawn + 1] == targetPce) {
            return TRUE;
        }
        else if (board->pieces[sqWithPawn - 1] == targetPce) {
            return TRUE;
        }
    }
    return FALSE;
}

U64 PolyKeyFromBoard(const S_BOARD *board) {
    int sq = 0, rank = 0, file = 0;
    U64 finalKey = 0;
    int piece = EMPTY;
    int polyPiece = 0;
    int offset = 0;

    for (sq = 0; sq < BRD_SQ_NUM; sq++) {
        piece = board->pieces[sq];
        if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {
            ASSERT(piece >= wP && piece <= bK);
            polyPiece = PolyKindOfPiece[piece];
            rank = RanksBrd[sq];
            file = FilesBrd[sq];
            finalKey ^= Random64Poly[(64 * polyPiece) + (8 * rank) + file]; 
        }
    }

    // castling
    offset = 768;
    if (board->castlePerm & WKCA) finalKey ^= Random64Poly[offset + 0];
    if (board->castlePerm & WQCA) finalKey ^= Random64Poly[offset + 1];
    if (board->castlePerm & BKCA) finalKey ^= Random64Poly[offset + 2];
    if (board->castlePerm & BQCA) finalKey ^= Random64Poly[offset + 3];

    // enpassant
    offset = 772;
    if (HasPawnForCapture(board) == TRUE) {
        file = FilesBrd[board->enPas];
        finalKey ^= Random64Poly[offset + file];
    }

    // side
    offset = 780;
    if (board->side == WHITE) {
        finalKey ^= Random64Poly[offset];
    }

    return finalKey;
}

unsigned short endian_swap_u16(unsigned short x) 
{ 
    x = (x>>8) | 
        (x<<8); 
    return x;
} 

unsigned int endian_swap_u32(unsigned int x) 
{ 
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) | 
        ((x>>8) & 0x0000FF00) | 
        (x<<24); 
    return x;
} 

U64 endian_swap_u64(U64 x) 
{ 
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) | 
        ((x<<24) & 0x0000FF0000000000) | 
        ((x<<8)  & 0x000000FF00000000) | 
        ((x>>8)  & 0x00000000FF000000) | 
        ((x>>24) & 0x0000000000FF0000) | 
        ((x>>40) & 0x000000000000FF00) | 
        (x<<56); 
    return x;
}

int ConvertPolyMoveToInternalMove(unsigned short polyMove, S_BOARD *board) {
    int ff = (polyMove >> 6) & 7; // from file is shifted by 7 bits
    int fr = (polyMove >> 9) & 7; // from file is shifted by 9 bits
    int tf = (polyMove >> 0) & 7; // to file is shifted by 7 bits
    int tr = (polyMove >> 3) & 7; // to file is shifted by 9 bits
    int pp = (polyMove >> 12) & 7; // promotion piece at 12, 13 and 14 bits

    char moveString[6];
    
    if(pp == 0) {
        sprintf(moveString, "%c%c%c%c",
                FileChar[ff],
                RankChar[fr],
                FileChar[tf],
                RankChar[tr]);
    } 
    else {
        char promChar = 'q';
        switch(pp) {
            case 1: promChar = 'n'; break;
            case 2: promChar = 'b'; break;
            case 3: promChar = 'r'; break;
        }
        sprintf(moveString, "%c%c%c%c%c",
                FileChar[ff],
                RankChar[fr],
                FileChar[tf],
                RankChar[tr],
                promChar);
    }
    return ParseMove(moveString, board);
}

int GetBookMove(S_BOARD *board ) {

    U64 PolyKey = PolyKeyFromBoard(board); // now we check if the key in entry matches with this

    int index = 0;
    S_POLY_BOOK_ENTRY *entry;
    unsigned short move;
    const int MAXBOOKMOVES = 100;
    int bookMoves[MAXBOOKMOVES];
    int tempMove = NOMOVE;
    int count = 0;

    for (entry = entries; entry < entries + NumEntries; ++entry) {
        if (PolyKey == endian_swap_u64(entry->key)) {
            move = endian_swap_u16(entry->move); // entries are stored in big endian and not little endian -> i.e instead of 8e3f it is 3f8e -> so we need to reverse it
            // printf("key:%llx, Index:%d, Move:%c%c%c%c\n", 
            //         endian_swap_u64(entry->key),
            //         index,
            //         FileChar[(move >> 6) & 7],
            //         RankChar[(move >> 9) & 7],
            //         FileChar[(move >> 0) & 7],
            //         RankChar[(move >> 3) & 7]);

            tempMove = ConvertPolyMoveToInternalMove(move, board);
            if (tempMove != NOMOVE) {
                bookMoves[count] = tempMove;
                count++;

                if (count > MAXBOOKMOVES) break;
            }
        }
    }
    if (count != 0) {
        int randMove = rand() % count;
        return bookMoves[randMove];
    }
    else {
        return NOMOVE;
    }
}