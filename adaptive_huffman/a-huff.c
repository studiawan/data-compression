
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitio.h"
#include "errhand.h"

char *CompressionName = "Adaptive Huffman coding, with escape codes";
char *Usage = "infile outfile";

#define END_OF_STREAM		256
#define ESCAPE				257
#define SYMBOL_COUNT		258
#define NODE_TABLE_COUNT	( ( SYMBOL_COUNT * 2 ) - 1 )
#define ROOT_NODE			0
#define MAX_WEIGHT			0X8000
#define TRUE				1
#define FALSE				0
