// xboard.c

#include <stdio.h>
#include <string.h>
#include "defs.h"

// new -> new game
// sd 8 -> depth 8
// usermove e2e4 -> user has made this move now engine's turn
// move e7e5 -> our move

int ThreeFoldRep(const S_BOARD *pos)
{
    int i = 0, r = 0;
    for (i = 0; i < pos->histPly; ++i)
    {
        if (pos->history[i].posKey == pos->posKey)
        {
            r++;
        }
    }
    return r;
}
// position is a draw if neither side can give mate
// itr returns false when it is not a draw by insufficient material
// otherwise it returs true(draw by insufficient material)
int DrawMaterial(const S_BOARD *pos)
{
    if (pos->pceNum[wP] || pos->pceNum[bP])
        return FALSE; // if pawn is there, it can promote and deliver mate
    if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR])
        return FALSE; // if major pieces present, mate can be delivered
    if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1)
        return FALSE; // if both the bishops are present the it can be mated
    if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1)
        return FALSE; // if both the knights are presnt, mate can be delivered
    if (pos->pceNum[wN] || pos->pceNum[wB])
        return FALSE; // mate can be delivered with knight and bishop
    if (pos->pceNum[bN] || pos->pceNum[bB])
        return FALSE; // ...

    return TRUE;
}
// checks if current state is a draw or not
int CheckResult(S_BOARD *pos)
{
    if (pos->fiftyMove > 100)
    {
        "1/2-1/2 {fifty move rule (claimed by Vince)}\n";
        return TRUE;
    }

    if (ThreeFoldRep(pos) >= 2)
    {
        "1/2-1/2 {3-fold repetition (claimed by Vince)}\n";
        return TRUE;
    }

    if (DrawMaterial(pos) == TRUE)
    {
        "1/2-1/2 {insufficient material (claimed by Vince)}\n";
        return TRUE;
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);
    int MoveNum = 0;
    int found = 0;

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
    {
        if (!MakeMove(pos, list->moves[MoveNum].move))
        {
            continue;
        }
        found++;
        TakeMove(pos);
        break;
    }

    if (found != 0)
    {
        return FALSE;
    }

    int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);

    if (InCheck == TRUE)
    {
        if (pos->side == WHITE)
        {
            printf("0-1 {black mates (claimed by Vince)}\n");
            return TRUE;
        }
        else
        {
            printf("0-1 {white mates (claimed by Vince)}\n");
            return TRUE;
        }
    }
    else
    {
        printf("\n1/2-1/2 {stalemate (claimed by Vince)}");
        return TRUE;
    }
    return FALSE;
}

void PrintOptions()
{
    printf("feature ping=1 setboard=1 colors=0 usermove=1\n");
    printf("feature done=1\n");
}

void XBOARD_Loop(S_BOARD *pos, S_SEARCHINFO *info)
{

    info->GAME_MODE = XBOARDMODE;
    info->POST_THINKING = TRUE;
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    PrintOptions(); // HACK

    int depth = -1, movestogo[2] = {30, 30}, movetime = -1;
    int time = -1, inc = 0;
    int engineSide = BOTH;
    int timeLeft;
    int sec;
    int mps;
    int move = NOMOVE;
    char inBuf[80], command[80];
    int sizeInMb;

    engineSide = BLACK;
    ParseFen(START_FEN, pos);
    depth = -1;
    time = -1;

    while (TRUE)
    {

        fflush(stdout);

        if (pos->side == engineSide && CheckResult(pos) == FALSE)
        {
            info->startTime = GetTimeMs();
            info->depth = depth;

            if (time != -1)
            {
                info->timeSet = TRUE;
                time /= movestogo[pos->side];
                // time -= 50;
                info->stopTime = info->startTime + time + inc;
            }

            if (depth == -1 || depth > MAXDEPTH)
            {
                info->depth = MAXDEPTH;
            }

            printf("time:%d start:%d stop:%d depth:%d timeSet:%d movestogo:%d mps:%d\n",
                   time, info->startTime, info->stopTime, info->depth, info->timeSet, movestogo[pos->side], mps);
            SearchPosition(pos, info, HashTable);

            if (mps != 0)
            {
                movestogo[pos->side ^ 1]--;
                if (movestogo[pos->side ^ 1] < 1)
                {
                    movestogo[pos->side ^ 1] = mps;
                }
            }
        }

        fflush(stdout);

        memset(&inBuf[0], 0, sizeof(inBuf));
        fflush(stdout);
        if (!fgets(inBuf, 80, stdin))
            continue;

        sscanf(inBuf, "%s", command);

        printf("command seen:%s\n", inBuf);

        if (!strcmp(command, "quit"))
        {
            info->quit = TRUE;
            break;
        }

        if (!strcmp(command, "force"))
        {
            engineSide = BOTH;
            continue;
        }

        if (!strcmp(command, "protover"))
        {
            PrintOptions();
            continue;
        }

        if (!strcmp(command, "sd"))
        {
            sscanf(inBuf, "sd %d", &depth);
            printf("DEBUG depth:%d\n", depth);
            continue;
        }

        if (!strcmp(command, "st"))
        {
            sscanf(inBuf, "st %d", &movetime);
            printf("DEBUG movetime:%d\n", movetime);
            continue;
        }

        if (!strcmp(command, "time"))
        {
            sscanf(inBuf, "time %d", &time);
            time *= 10;
            printf("DEBUG time:%d\n", time);
            continue;
        }

        if(!strcmp(command, "memory")) {
        	sscanf(inBuf, "memory %d", &sizeInMb);
            if(sizeInMb < 4) sizeInMb = 4;
        	printf("Set Hash to %d Mb\n",sizeInMb);
        	InitHashTable(HashTable, sizeInMb);
        	continue;
        }

        if (!strcmp(command, "level"))
        {
            sec = 0;
            movetime = -1;
            if (sscanf(inBuf, "level %d %d %d", &mps, &timeLeft, &inc) != 3)
            {
                sscanf(inBuf, "level %d %d:%d %d", &mps, &timeLeft, &sec, &inc);
                printf("DEBUG level with :\n");
            }
            else
            {
                printf("DEBUG level without :\n");
            }
            timeLeft *= 60000;
            timeLeft += sec * 1000;
            movestogo[0] = movestogo[1] = 30;
            if (mps != 0)
            {
                movestogo[0] = movestogo[1] = mps;
            }
            time = -1;
            printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps%d\n", timeLeft, movestogo[0], inc, mps);
            continue;
        }

        if (!strcmp(command, "ping"))
        {
            printf("pong%s\n", inBuf + 4);
            continue;
        }

        if (!strcmp(command, "new"))
        {
            ClearHashTable(HashTable);
            engineSide = BLACK;
            ParseFen(START_FEN, pos);
            depth = -1;
            time = -1;
            continue;
        }

        if (!strcmp(command, "setboard"))
        {
            engineSide = BOTH;
            ParseFen(inBuf + 9, pos);
            continue;
        }

        if (!strcmp(command, "go"))
        {
            engineSide = pos->side;
            continue;
        }

        if (!strcmp(command, "usermove"))
        {
            movestogo[pos->side]--;
            move = ParseMove(inBuf + 9, pos);
            if (move == NOMOVE)
                continue;
            MakeMove(pos, move);
            pos->ply = 0;
        }
        if (!strcmp(command, "print"))
        {
            PrintBoard(pos);
        }
    }
}

void Console_Loop(S_BOARD *pos, S_SEARCHINFO *info)
{
    printf("Welcome to Vice In Console Mode!\n");
    printf("Type help for commands\n\n");

    info->GAME_MODE = CONSOLEMODE;
    info->POST_THINKING = TRUE;
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    int depth = MAXDEPTH, movetime = 3000;
    int engineSide = BOTH;
    int move = NOMOVE;
    char inBuf[80], command[80];
    int perftDepth = 5;
    char *perftOn;

    engineSide = BLACK;
    ParseFen(START_FEN, pos);

    while (TRUE)
    {

        fflush(stdout);

        if (pos->side == engineSide && CheckResult(pos) == FALSE)
        {
            info->startTime = GetTimeMs();
            info->depth = depth;

            if (movetime != 0)
            {
                info->timeSet = TRUE;
                info->stopTime = info->startTime + movetime;
            }

            SearchPosition(pos, info, HashTable);
        }

        printf("\nVince > ");

        fflush(stdout);

        memset(&inBuf[0], 0, sizeof(inBuf));
        fflush(stdout);
        if (!fgets(inBuf, 80, stdin))
            continue;

        sscanf(inBuf, "%s", command);

        if (!strcmp(command, "help"))
        {
            printf("Commands:\n");
            printf("quit - quit game\n");
            printf("force - computer will not think\n");
            printf("print - show board\n");
            printf("post - show thinking\n");
            printf("nopost - do not show thinking\n");
            printf("new - start new game\n");
            printf("go - set computer thinking\n");
            printf("go perft x on fenstring (x = depth)");
            printf("depth x - set depth to x\n");
            printf("time x - set thinking time to x seconds (depth still applies if set)\n");
            printf("view - show current depth and movetime settings\n");
            printf("setboard x - set position to fen x\n");
            printf("** note ** - to reset time and depth, set to 0\n");
            printf("enter moves using b7b8q notation\n\n\n");
            continue;
        }

        if (!strcmp(command, "mirrorpos"))
        {
            engineSide = BOTH;
            PrintBoard(pos);
            MirrorBoard(pos);
            PrintBoard(pos);
            MirrorBoard(pos);
            continue;
        }
        
        else if (!strcmp(command, "mirror"))
        {
            engineSide = BOTH;
            MirrorEvalTest(pos);
            continue;
        }

        if (!strcmp(command, "eval"))
        {
            PrintBoard(pos);
            printf("Eval:%d", EvalPosition(pos));
            MirrorBoard(pos);
            PrintBoard(pos);
            printf("Eval:%d", EvalPosition(pos));
            continue;
        }

        if (!strcmp(command, "setboard"))
        {
            engineSide = BOTH;
            ParseFen(inBuf + 9, pos);
            continue;
        }

        if (!strcmp(command, "quit"))
        {
            info->quit = TRUE;
            break;
        }

        if (!strcmp(command, "post"))
        {
            info->POST_THINKING = TRUE;
            continue;
        }

        if (!strcmp(command, "print"))
        {
            PrintBoard(pos);
            continue;
        }

        if (!strcmp(command, "nopost"))
        {
            info->POST_THINKING = FALSE;
            continue;
        }

        if (!strcmp(command, "force"))
        {
            engineSide = BOTH;
            continue;
        }

        if (!strcmp(command, "view"))
        {
            if (depth == MAXDEPTH)
                printf("depth not set ");
            else
                printf("depth %d", depth);

            if (movetime != 0)
                printf(" movetime %ds\n", movetime / 1000);
            else
                printf(" movetime not set\n");
            if (EngineOptions->UseBook == TRUE) {
                printf("Use book enabled\n");
            }
            else {
                printf("Book not allowed\n");
            }

            continue;
        }

        if (!strcmp(command, "depth"))
        {
            sscanf(inBuf, "depth %d", &depth);
            if (depth == 0)
                depth = MAXDEPTH;
            continue;
        }

        if (!strcmp(command, "time"))
        {
            sscanf(inBuf, "time %d", &movetime);
            movetime *= 1000;
            continue;
        }

        if (!strcmp(command, "new"))
        {
            ClearHashTable(HashTable);
            engineSide = BLACK;
            ParseFen(START_FEN, pos);
            continue;
        }

        if (!strcmp(command, "go"))
        {
            engineSide = pos->side;
            continue;
        }

        if (!strcmp(command, "perft")) {
            PerftTest(perftDepth, pos);
            continue;
        }
        if (!strcmp(command, "perftdepth")) {
            sscanf(inBuf, "perftdepth %d", &perftDepth);
            continue;
        }

        move = ParseMove(inBuf, pos);
        if (move == NOMOVE)
        {
            printf("Command unknown:%s\n", inBuf);
            continue;
        }
        MakeMove(pos, move);
        pos->ply = 0;
    }
}