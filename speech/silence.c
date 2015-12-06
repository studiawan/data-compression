/************************ Start of SILENCE.C ************************
*
* This is the silence compression coding module used in chapter 10.
* Compile with BITIO.C, ERRHAND.C, and either MAIN-C.C or MAIN-E.C
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lib/bitio.h"
#include "../lib/errhand.h"
#include "../lib/main.h"

/*
* These two strings are used by MAIN-C.C and MAIN-E.C to print
* messages of importance to the user of the program.
*/
char *CompressionName = "Silence compression";
char *Usage = "infile outfile\n";
/*
* These macros define the parameters used to compress the silent
* sequences. SILENCE_LIMIT is the maximum size of a signal that can
* be considered silent, in terms of offset from the center point.
* START_THRESHOLD gives the number of consecutive silent codes that
* have to be seen before a run is started. STOP_THRESHOLD tells how
* many non-silent codes need to be seen before a run is considered
* to be over. SILENCE_CODE is the special code output to the
* compressed file to indicate that a run has been detected.
* SILENCE_CODE is always followed by a single byte indicating how
* many consecutive silence bytes are to follow.
*/

#define SILENCE_LIMIT 4
#define START_THRESHOLD 5
#define STOP_THRESHOLD 2
#define SILENCE_CODE 0xff
#define IS_SILENCE( c ) ( (c) >( 0x7f - SILENCE_LIMIT ) && \
                         (c) < ( 0x80 + SILENCE_LIMIT ) )
/*
* BUFFER_SIZE is the size of the look-ahead buffer. BUFFER_MASK is
* the mask applied to a buffer index when performing index math.
*/
#define BUFFER_SIZE 8
#define BUFFER_MASK 7

/*
* Local function prototypes.
*/

#ifdef __STDC__

int silence_run( int buffer[], int index );
int end_of_silence( int buffer[], int index);

#else

int silence_run();
int end_of_silence();

#endif // __STDC__

/*
* The compression routine has the hard job here. It has to detect
* when a silence run has started and when it is over. It does this
* by keeping up-and-coming bytes in a look-ahead buffer. The buffer
* and the current index are passed ahead to routines that check to
* see if a run has started or if it has ended.
*/

void CompressFile( FILE *input, BIT_FILE *output, int argc, char *argv[] ){
/*
FILE *input;
BIT_FILE *output;
int argc;
char *argv[];
{
*/
    int look_ahead[ BUFFER_SIZE ];
    int index;
    int i;
    int run_length;

    for ( i = 0 ; i < BUFFER_SIZE ; i++ )
        look_ahead[ i ] = getc( input );
        index = 0;
    for ( ; ; ) {
        if ( look_ahead[ index ] == EOF )
            break;
/*
* If run has started, I handle it here. I sit in the do loop until
* the run is complete, loading new characters all the while.
*/

    if ( silence_run( look_ahead, index ) ) {
        run_length = 0;
        do {
            look_ahead[ index ++ ] = getc( input );
            index &= BUFFER_MASK;
            if ( ++run_length == 255 ) {
                putc( SILENCE_CODE, output->file );
                putc( 255, output->file );
                run_length = 0;
            }
        } while ( !end_of_silence( look_ahead, index ) );
        if ( run_length > 0 ) {
            putc( SILENCE_CODE, output->file );
            putc( run_length, output->file );
        }
    }
/*
* Eventually, any run of silence is over, and I output some plain codes.
* Any code that accidentally matches the silence code gets silently
* changed.
*/
    if ( look_ahead[ index ]== SILENCE_CODE )
        look_ahead[ index ]--;
    putc( look_ahead[ index ], output->file );
    look_ahead[ index++ ] = getc( input );
    index = BUFFER_MASK;
    }
    while ( argc-- > 0 )
        printf( "Unused argument: %s\n", *argv++ );
}
/*
* The expansion routine used here has a very easy time of it. It just
* has to check for the run code, and when it finds it, pad out the
* output file with some silence bytes.
*/
void ExpandFile( BIT_FILE *input, FILE *output, int argc, char *argv[] ){
/*
BIT_FILE *input;
FILE *output;
int argc;
char argv[];
{
*/
    int c;
    int run_count;

    while ( ( c = getc( input->file ) ) != EOF ) {
        if ( c == SILENCE_CODE ) {
            run_count = getc( input->file );
            while ( run_count-- > 0 )
                putc( 0x80, output );
        } else
            putc( c, output );
    }
    while ( argc-- > 0 )
        printf( "argumen tidak terpakai: %s\n", *argv++ );
}
/*
* This support routine checks to see if the look-ahead buffer
* contains the start of a run, which by definition is
* START_THRESHOLD consecutive silence characters.
*/
int silence_run( int buffer[], int index ){
/*
int buffer[];
int index;
{
*/
    int i;

    for ( i = 0 ; i < START_THRESHOLD ; i++ )
        if ( !IS_SILENCE( buffer[ ( index + i ) & BUFFER_MASK ] ) )
            return( 0 );
    return( 1 );
}
/*
* This support routine is called while we are in the middle of a
* run of silence. It checks to see if we have reached the end of
* the run. By definition this occurs when we see STOP_THRESHOLD
* consecutive non-silence characters.
*/
int end_of_silence( int buffer[], int index ){
/*
int buffer[];
int index;
{
*/
    int i;

    for ( i = 0 ; i < STOP_THRESHOLD ; i++ )
        if ( IS_SILENCE( buffer[ ( index + i ) & BUFFER_MASK ] ) )
            return( 0 );
    return( 1 );
}
/************************ End of SILENCE.C ************************/
