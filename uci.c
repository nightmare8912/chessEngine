// uci.c

#include <stdio.h>
#include <string.h>
#include "defs.h"

#define INPUTBUFFER 400 * 6

void ParseGo(char *line, S_SEARCHINFO *info, S_BOARD *pos)
{
    // go depth 6 wtime 18000 btime 10000 binc 100 winc 1000 movetime 1000 movestogo 40
    int depth = -1, movestogo = 30, movetime = -1;
    int time = -1, inc = 0;
    char *ptr = NULL;
    info->timeSet = FALSE;

    if ((ptr = strstr(line, "infinite")))
    {
        ;
    }

    if ((ptr = strstr(line, "binc")) && pos->side == BLACK)
    {
        inc = atoi(ptr + 5);
    }

    if ((ptr = strstr(line, "winc")) && pos->side == WHITE)
    {
        inc = atoi(ptr + 5);
    }

    if ((ptr = strstr(line, "wtime")) && pos->side == WHITE)
    {
        time = atoi(ptr + 6);
    }

    if ((ptr = strstr(line, "btime")) && pos->side == BLACK)
    {
        time = atoi(ptr + 6);
    }

    if ((ptr = strstr(line, "movestogo")))
    {
        movestogo = atoi(ptr + 10);
    }

    if ((ptr = strstr(line, "movetime")))
    {
        movetime = atoi(ptr + 9);
    }

    if ((ptr = strstr(line, "depth")))
    {
        depth = atoi(ptr + 6);
    }

    if (movetime != -1)
    {
        time = movetime;
        movestogo = 1;
    }

    info->startTime = GetTimeMs();
    info->depth = depth;

    if (time != -1)
    {
        info->timeSet = TRUE;
        time /= movestogo;
        // time -= 50; // we're keeping a margin of 50ms just to make sure that we don't overrun on time and stay behing prescribed time!
        info->stopTime = info->startTime + time + inc;
    }

    if (depth == -1)
    {
        info->depth = MAXDEPTH;
    }

    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
           time, info->startTime, info->stopTime, info->depth, info->timeSet);
    SearchPosition(pos, info);
}

void ParsePosition(char *lineIn, S_BOARD *pos)
{
    // position fen
    // position startpos
    // ... moves e2e4 ...

    lineIn += 9; // position word is of 8 char and the space so total is 9
    char *ptrChar = lineIn;
    if (strncmp(lineIn, "startpos", 8) == 0)
    {
        ParseFen(START_FEN, pos);
    }
    else
    {
        ptrChar = strstr(lineIn, "fen"); // strstr shifts the pointer to the given string
        if (ptrChar == NULL)
        {
            ParseFen(START_FEN, pos);
        }
        else
        {
            ptrChar += 4;
            ParseFen(ptrChar, pos);
        }
    }

    ptrChar = strstr(lineIn, "moves"); // shifting the pointer char to "moves"
    int move;
    if (ptrChar != NULL)
    {
        ptrChar += 6; // LEN(moves) = 5 + 1(space)
        while (*ptrChar)
        {
            move = ParseMove(ptrChar, pos);
            if (move == NOMOVE)
                break;
            MakeMove(pos, move);
            pos->ply = 0;
            while (*ptrChar && *ptrChar != ' ')
                ptrChar++;
            ptrChar++;
        }
    }
    PrintBoard(pos);
}

void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info)
{
    info->GAME_MODE = UCIMODE;
    int sizeInMb = 16;
    
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char line[INPUTBUFFER];

    printf("id name %s\n", NAME);
    printf("id author PRATYUSH\n");
    printf("uciok\n");

    while (TRUE)
    {
        memset(&line[0], 0, sizeof(line));
        fflush(stdout);
        if (!fgets(line, INPUTBUFFER, stdin))
        {
            continue;
        }

        if (line[0] == '\n')
        {
            continue;
        }

        if (!strncmp(line, "isready", 7))
        {
            printf("readyok\n");
            continue;
        }
        else if (!strncmp(line, "position", 8))
        {
            ParsePosition(line, pos);
        }
        else if (!strncmp(line, "ucinewgame", 10))
        {
            ParsePosition("position startpos\n", pos);
        }
        else if (!strncmp(line, "go", 2))
        {
            ParseGo(line, info, pos);
        }
        else if (!strncmp(line, "quit", 4))
        {
            info->quit = TRUE;
            break;
        }
        else if (!strncmp(line, "uci", 3))
        {
            printf("id name %s\n", NAME);
            printf("id author PRATYUSH\n");
            printf("uciok\n");
        }
        else if(!strncmp(line, "debug", 4)) {
            DebugAnalysisTest(pos, info);
            break;
        }
        else if (!strncmp(line, "set option name Hash value ", 26)) {
            sscanf(line, "%*s %*s %*s %*s %d", &sizeInMb);
            if (sizeInMb < 4) sizeInMb = 4;
            printf("Set hash to: %d MB\n", sizeInMb);
            InitHashTable(pos->HashTable, sizeInMb);
        }
        else if (!strncmp(line, "setoption name Book value ", 26)) {
            char *ptrTrue = NULL;
            ptrTrue = strstr(line, "true");
            if (ptrTrue != NULL) {
               EngineOptions->UseBook = TRUE;
            }
            else {
                EngineOptions->UseBook = FALSE;
            }
        }
        
        if (info->quit)
            break;
    }
    // free(pos->PvTable->pTable);
}