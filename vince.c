#include <stdio.h>
#include "defs.h"

int main()
{
    printf("hi");
	AllInit();
	
	int PieceOne = rand();
	int PieceTwo = rand();
	int PieceThree = rand();
	int PieceFour = rand();

	printf("PieceOne: %X\n", PieceOne);
	printf("PieceTwo: %X\n", PieceTwo);
	printf("PieceThree: %X\n", PieceThree);
	printf("PieceFour: %X\n", PieceFour);\

	int key = PieceOne ^ PieceTwo ^ PieceThree ^ PieceFour;
	int TempKey = PieceOne;
	TempKey ^= PieceThree;
	TempKey ^= PieceFour;
	TempKey ^= PieceTwo;

	printf("Key: %X\n", key);
	printf("TempKey: %X\n", TempKey);

    return 0;
}