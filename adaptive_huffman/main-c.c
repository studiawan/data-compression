
/***********************Start of MAIN-C.C***********************
* This driver program tests compression algorithms. To cut back on
* repetitive code, this version of main is used with all the expansion
* routines. The main() routine supplied here checks for valid input and
* output files, opens them, then calls the compression routine.
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

int main(int argc, char *argv[])
{
	BIT_FILE *output;
	FILE *input;

	setbuf( stdout, NULL );
	if ( argc < 3 )
		usage_exit( argv[ 0 ] );
	input = fopen( argv[ 1 ], "r" );
	if ( input == NULL )
		fatal_error( "Error opening %s for input %d \n", argv[ 1 ],'\n');
	output = OpenOutputBitFile( argv[ 2 ] );
	if ( output == NULL )
		fatal_error( "Error opening %s for output\n", argv[ 2 ]);
	printf( "\nCompressing %s to %s\n", argv[ 1 ], argv[ 2 ]);
	printf( "Using %s\n", CompressionName );
	argc -= 3;
	argv += 3;
	CompressFile( input, output, argc, argv );
	fclose( input );
	CloseOutputBitFile( output );
	putc( '\n', stdout );
	return( 0 );
}

/*
* This routine wants to print out the usage message called for when the
* program is run with no parameters. The first part of the Usage state
* ment is supposed to be just the programname. argv[ 0 ] generally holds
* the fully qualified path name of the program being run.
*/
void usage_exit(char *prog_name)
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
	printf( "\nUsage: %s %s\n", short_name, Usage );
	exit( 0 );
}
/********************** End of MAIN-C.C**************************/
