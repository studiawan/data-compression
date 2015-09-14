/*************************** Start of GS.C *****************************
*
* This is the GS program, which displays grey-scale files on the
* IBM VGA adaptor. It assumes that the grey-scale values run from
* zero to 255, and scales them down to a range of zero to sixty-three,
* so they will be displayed properly on the VGA.
*
* This program can be called with a list of files, and will display them
* in consecutive order, which is useful for trying to measure visual
* differences in compressed files.
*
* This program writes directly to video memory, which should work
* properly on most VGA adaptors. In the event that it doesn't, the
* constant USE_BIOS can be turned on, and the code will use BIOS calls
* to write pixels instead. This will be somewhat slower, but should work
* on every VGA adaptor.
*
* Note that the use of far pointers means this program should probably
* be compiled without using the strict ANSI option of your compiler.
*/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
