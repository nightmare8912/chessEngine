// validate.c

#include <string.h>
#include "defs.h"

int MoveListOk(const S_MOVELIST *list,  const S_BOARD *pos) {
	if(list->count < 0 || list->count >= MAXPOSITIONMOVES) {
		return FALSE;
	}

	int MoveNum;
	int from = 0;
	int to = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		to = TOSQ(list->moves[MoveNum].move);
		from = FROMSQ(list->moves[MoveNum].move);
		if(!SqOnBoard(to) || !SqOnBoard(from)) {
			return FALSE;
		}
		if(!PieceValid(pos->pieces[from])) {
			PrintBoard(pos);
			return FALSE;
		}
	}

	return TRUE;
}

int SqOnBoard(const int sq) {
    return FilesBrd[sq] == OFFBOARD ? 0 : 1;
}

int SideValid(const int side) {
    return (side == WHITE || side == BLACK) ? 1 : 0;
}

int FileRankValid(const int fr) {
    return (fr >= 0 && fr <= 7) ? 1 : 0; 
}

int PieceValidEmpty(const int pce) {
    return (pce >= EMPTY && pce <= bK) ? 1 : 0;
}

int PieceValid(const int pce) {
    return (pce >= wP && pce <= bK) ? 1 : 0; 
}

void MirrorEvalTest(S_BOARD *pos) {
    FILE *file;
    file = fopen("mirror.epd","r");
    char lineIn [1024];
    int ev1 = 0; int ev2 = 0;
    int positions = 0;
    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        int counter = 0;
        while(fgets (lineIn , 1024 , file) != NULL) {
            ParseFen(lineIn, pos);
            counter++;
            positions++;
            ev1 = EvalPosition(pos);
            MirrorBoard(pos);
            ev2 = EvalPosition(pos);
            if(ev1 != ev2) {
                printf("\n\n\n");
                ParseFen(lineIn, pos);
                PrintBoard(pos);
                MirrorBoard(pos);
                PrintBoard(pos);
                printf("\n\nMirror Fail at %d :\n%s\n", counter, lineIn);
                getchar();
                // return;
            }

            if( (positions % 1000) == 0)   {
                printf("position %d\n",positions);
            }

            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}

void DebugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {

	FILE *file;
    file = fopen("lct2.epd","r");
    char lineIn [1024];

	info->depth = 5;
	info->timeSet = TRUE;
	int time = 1140000;


    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
			info->startTime = GetTimeMs();
			info->stopTime = info->startTime + time;
			ClearHashTable(table);
            ParseFen(lineIn, pos);
            printf("\n%s\n",lineIn);
			printf("time:%d start:%d stop:%d depth:%d timeSet:%d\n",
				time,info->startTime,info->stopTime,info->depth,info->timeSet);
			SearchPosition(pos, info, table);
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}