/**************************** Start of CARMAN.C *************************
*
* This is the main program for the simple Compressed Archive Manager.
* This program can be used to add, delete, extract, or list the files
* in a CAR archive. The code here should run under standard ANSI
* compilers under MS-DOS (with ANSI mode selected) or K&R compilers
* under UNIX. The code uses an LZSS compression algorithm identical to
* that used earlier in the book.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __STDC__
#include <stdarg.h>
#endif
