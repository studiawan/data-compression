#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitio.h"
#include "errhand.h"
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
