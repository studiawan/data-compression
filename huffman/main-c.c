/*********************** Start of MAIN-C.C **********************/
/*
* This is the driver program used when testing compression algorithms.
* In order to cut back on repetitive code, this version of main is
* used with all of the compression routines. In order to turn it into
* a real program, it needs to have another module that supplies one
* routine and two strings, namely:
*
*
void CompressFile( FILE *input, BIT_FILE *output,
*
int argc, char *argv );
*
char *Usage;
*
char *CompressionName;
*
* The main() routine supplied here has the job of checking for valid
* input and output files, opening them, and then calling the
* compression routine. If the files are not present, or no arguments
* are supplied, it prints out an error message, which includes the
* Usage string supplied by the compression module. All of the
* routines and strings needed by this routine are defined in the
* main.h header file.
*
* After this is built into a compression program of any sort, the
* program can be called like this:
*
*
main-c infile outfile [ options ]
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/bitio.h"
#include "../lib/errhand.h"
#include "main.h"
#ifdef __STDC__

void usage_exit( char *prog_name );
void print_ratios( char *input, char *output );
long file_size( char *name );

#else
void usage_exit();
void print_ratios();
long file_size();

#endif

int main( argc, argv )
int argc;
char *argv[];
{
	BIT_FILE *output;
	FILE *input;
	
	setbuf( stdout, NULL );
	if ( argc < 3 )
		usage_exit( argv[ 0 ] );
	input = fopen(argv [ 1 ], "rb" );
	if ( input == NULL )
		fatal_error( "Error opening %s for input\n", argv[ 1 ] );
	output = OpenOutputBitFile( argv[ 2 ] );
	if ( output == NULL )
		fatal_error( "Error opening %s for output\n", argv[ 2 ] );
	printf( "\nCompressing %s to %s\n", argv[ 1 ], argv[ 2 ] );
	printf( "Using %s\n", CompressionName );
	CompressFile( input, output, argc - 3, argv + 3 );
	CloseOutputBitFile( output );
	fclose( input );
	print_ratios( argv[ 1 ], argv[ 2 ] );
	return( 0 );
}

/*
* This routine just wants to print out the usage message that is
* called for when the program is run with no parameters. The first
* part of the Usage statement is supposed to be just the program
* name. argv[ 0 ] generally holds the fully qualified path name
* of the program being run. I make a half-hearted attempt to strip
* out that path info and file extension before printing it. It should
* get the general idea across.
*/
void usage_exit( prog_name )
char *prog_name;
{
	char *short_name;
	char *extension;
	
	short_name = strrchr( prog_name, '\\' );
	if (short_name == NULL )
		short_name = strrchr( prog_name, '/' );
	if (short_name == NULL )
		short_name = strrchr( prog_name, ':' );
	if (short_name != NULL )
		short_name++;
	else
		short_name = prog_name;
	extension = strrchr( short_name, '.' );
	if ( extension != NULL )
		*extension = '\0';
	printf( "\nUsage: %s %s\n", short_name, Usage );
	exit( 0 );
}

/*
* This routine is used by main to get the size of a file after it has
* been closed. It does all the work, and returns a long. The main
* program gets the file size for the plain text, and the size of the
* compressed file, and prints the ratio.
*/
#ifndef SEEK_END
#define SEEK_END 2
#endif
long file_size( name )
char *name;
{
	long eof_ftell;
	FILE *file;
	
	file = fopen( name, "r");
	if ( file == NULL )
		return( 0L );
	fseek( file, 0L, SEEK_END );
	eof_ftell = ftell( file );
	fclose( file );
	return( eof_ftell );
}

/*
* This routine prints out the compression ratios after the input and
* output files have been closed.
*/
void print_ratios( input, output )
char *input;
char *output;
{
	long input_size;
	long output_size;
	int ratio;
	input_size = file_size( input );
	if ( input_size == 0 )
		input_size = 1;
	output_size = file_size ( output );
	ratio = 100 - (int) ( output_size * 100L / input_size );
	printf( "\nInput bytes:	%ld\n", input_size );
	printf( "Output bytes:	%ld\n", output_size );
	if ( output_size == 0 )
		output_size = 1;
	printf( "Compression ratio: %d%%\n", ratio );
}
/*********************** End of MAIN-C.C *************************/



