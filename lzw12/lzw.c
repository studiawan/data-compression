/************************* Start of LZW12.C **************************
*
* This is 12 bit LZW program, which is discussed in the first part
* of the chapter. It uses a fixed size code, and does not attempt
* to flush the dictionary after it fills up.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errand.h"
#include "bitio.h"

