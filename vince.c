#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "sample_fens.h"

int main()
{
	AllInit();

	S_BOARD pos[1];
    S_SEARCHINFO info[1];
    InitPvTable(pos->PvTable);

	printf("Welcome to Vince! Type 'vince' for console mode\n\n");
	char line[256];

	while(TRUE) {
		memset(&line[0], 0, sizeof(line));
		fflush(stdout);

		if (!fgets(line, 256, stdin)) {
			continue;
		}
		if (line[0] == '\n') {
			continue;
		}
		if (!strncmp(line, "uci", 3)) {
			Uci_Loop(pos, info);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (!strncmp(line, "xboard", 6)) {
			XBOARD_Loop(pos, info);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (!strncmp(line, "vince", 5)) {
			Console_Loop(pos, info);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (!strncmp(line, "quit", 4)) {
			break;
		}
	}

	free(pos->PvTable->pTable);

    return 0;
}
