#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "sample_fens.h"

int main(int argc, char *argv[])
{
	AllInit();

	S_BOARD pos[1];
    S_SEARCHINFO info[1];
    // InitPvTable(pos->PvTable);
	info->quit = FALSE;
	info->threadNum = 4;
	HashTable->pTable = NULL;
	InitHashTable(HashTable, 16);

	printf("Welcome to Vince! Type 'vince' for console mode\n\n");
	char line[256];

	int argNum = 0;

	for (argNum = 0; argNum < argc; argNum++) {
		if (strncmp(argv[argNum], "nobook", 6) == 0) {
			EngineOptions->UseBook = FALSE;
			printf("Book off!\n");
		}
	}

	// TempHashTest(FEN18);
	// TempHashTest(FEN4);
	// exit(0);

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
			Uci_Loop(pos, info, HashTable);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (!strncmp(line, "xboard", 6)) {
			XBOARD_Loop(pos, info, HashTable);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (!strncmp(line, "vince", 5)) {
			Console_Loop(pos, info, HashTable);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (!strncmp(line, "quit", 4)) {
			break;
		}
	}

	// free(pos->PvTable->pTable);
	free(HashTable->pTable);
	ClearPolyBook();
    return 0;
}
