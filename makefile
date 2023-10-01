all:
	gcc vince.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c movegen.c validate.c makemove.c perft.c search.c misc.c pvtable.c evaluate.c uci.c xboard.c polybook.c polykeys.c -o vince