/************************** Start of LZW15V.C ***************************
*
* This is the LZW module which implements a more powerful version
* of the algorithm. This version of the program has three major
* improvements over LZW12.C. First, it expands the maximum code size
* to 15 bits. Second, it starts encoding with 9 bit codes, working
* its way up in bit size only as necessary. Finally, it flushes the
* dictionary when done.
*
* Note that under MS-DOS this program needs to be built using the
* Compact or Large memory model.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errhand.h"
#include "bitio.h"
