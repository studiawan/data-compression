/************************** Start of MAIN-E.C *************************
 *
 * This is the driver program used when testing compression algorithms.
 * In order to cut back on repetitive code, this version of main is used
 * with all of the expansion routines.  It in order to turn into a real
 * program, it needs to have another module that supplies a routine and
 * two strings, namely:
 *
 *     void ExpandFile( BIT_FILE *input, FILE *output, int argc, char *argv );
 *     char *Usage;
 *     char *CompressionName;
 *
 * The main() routine supplied here has the job of checking for valid
 * input and output files, opening them, and then calling the compression
 * routine.  If the files are not present, or no arguments are supplied,
 * it calls the Usage() routine, which is expected to print out
 * the compression type.  All of these routines are defined in the main.h
 * header file.
 *
 * After this is built into an expansion program of any sort, the program
 * can be called like this:
 *
 *   expand infile outfile [ options ]
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitio.h"
#include "errhand.h"
#include "main.h"

#ifdef __STDC__

void usage_exit( char *prog_name );

#else

void usage_exit();

#endif

int main( argc, argv )
int argc;
char *argv[];
{
    FILE *output;
    BIT_FILE *input;

    setbuf( stdout, NULL );
    if ( argc < 3 )
	usage_exit( argv[ 0 ] );
    input = OpenInputBitFile( argv[ 1 ] );
    if ( input == NULL )
	fatal_error( "Error opening %s for input\n", argv[ 1 ] );
    output = fopen( argv[ 2 ], "wb" );
    if ( output == NULL )
	fatal_error( "Error opening %s for output\n", argv[ 2 ] );
    printf( "\nExpanding %s to %s\n", argv[ 1 ], argv[ 2 ] );
    printf( "Using %s\n", CompressionName );
    argc -= 3;
    argv += 3;
    ExpandFile( input, output, argc, argv );
    CloseInputBitFile( input );
    fclose( output );
    putc( '\n', stdout );
    return( 0 );
}

/*
 * This routine just wants to print out the usage message that is
 * called for when the program is run with no parameters.  The first
 * part of the Usage statement is supposed to be just the program
 * name.  argv[ 0 ] generally holds the fully qualified path name
 * of the program being run.  I make a half-hearted attempt to strip
 * out that path info before printing it.  It should get the general
 * idea across.
 */
void usage_exit( prog_name )
char *prog_name;
{
    char *short_name;
    char *extension;

    short_name = strrchr( prog_name, '\\' );
    if ( short_name == NULL )
	short_name = strrchr( prog_name, '/' );
    if ( short_name == NULL )
	short_name = strrchr( prog_name, ':' );
    if ( short_name != NULL )
	short_name++;
    else
	short_name = prog_name;
    extension = strrchr( short_name, '.' );
    if ( extension != NULL )
	*extension = '\0';
    printf( "\nUsage:  %s %s\n", short_name, Usage );
    exit( 0 );
}

/************************** Start of MAIN-E.C *************************/


