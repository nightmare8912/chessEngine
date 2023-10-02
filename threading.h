#ifndef THREADING_H
#define THREADING_H

#include "defs.h"
#include "tinycthread.h"

static thrd_t mainSearchThread;

static thrd_t LaunchSearchThread(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {
    
    S_SEARCH_THREAD_DATA *pSearchData = malloc(sizeof(S_SEARCH_THREAD_DATA));

    pSearchData->originalPosition = pos;
    pSearchData->info = info;
    pSearchData->ttable = table;

    thrd_t th;
    thrd_create(&th, &SearchPositionThread, (void *)pSearchData);

    return th;
}

static void JoinSearchThread(S_SEARCHINFO *info) {
    info->stopped = TRUE;
    thrd_join (mainSearchThread, NULL);
}

static void PerftThread(S_BOARD *pos, int depth) {
    
}

#endif