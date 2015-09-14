/*********************** Start of LZSS.C ***********************
*
* This is the LZSS module, which implements an LZ77 style compression
* algorithm. As implemented here it uses a 12 bit index into the sliding
* window, and a 4 bit length, which is adjusted to reflect phrase
* lengths of between 2 and 17 bytes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitio.h"

