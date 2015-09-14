/***************************** Start of GSDIFF.C ***********************
*
* This is the GSDIFF program, which displays the differences between
* two grey-scale files on the IBM VGA adaptor. It assumes that the
* grey-scale values run from zero to 255, and scales them down to a
* range of zero to sixty-three, so they will be displayed properly on
* the VGA.
*
* This program writes directly to video memory, which should work
* properly on most VGA adaptors. In the event that it doesn't, the
* constant USE_BIOS can be turned on, and the code will use BIOS calls
* to write pixels instead. This will be somewhat slower, but should work
* on every VGA adaptor.
*
* While this program is writing out to the display, it is also keeping a
* running total of the error differences. When the program is
* complete, it prints out the RMS error. If the -B switch is turned
* on, the program operates in batch mode, and doesn't display the
* differences. It just computes and prints the rms error value.
*
* Note that the use of far pointers means this program should probably
* be compiled without using the strict ANSI option of your compiler.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <dos.h>
#include <conio.h>
#include <math.h>
