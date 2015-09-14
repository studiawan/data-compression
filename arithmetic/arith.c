#include <stdio.h>
#include <stdlib.h>
#include "errhand.h"
#include "bitio.h"

/*
* The SYMBOL structure is what is used to define a symbol in
* arithmetic coding terms. A symbol is defined as a range between
* 0 and 1. Since we are using integer math, instead of using 0 and 1
* as our end points, we have an integer scale. The low_count and
* high_count define where the symbol falls in the range.
*/

typedef struct {
	unsigned short int low_count;
	unsigned short int high_count;
	unsigned short int scale;
} SYMBOL;
