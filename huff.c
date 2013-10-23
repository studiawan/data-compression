/********************** Start of HUFF.C *************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitio.h"
#include "errhand.h"
#include "main.h"

/*
* The NODE structure is a node in the Huffman decoding tree. It has a
* count, which is its weight in the tree, and the node numbers of its
* two children. The saved_count member of the structure is only
* there for debugging purposes, and can be safely taken out at any
* time. It just holds the intial count for each of the symbols, since
* the count member is continually being modified as the tree grows.
*/
typedef struct tree_node {
	unsigned int count;
	unsigned int saved_count;
	int child_0;
	int child_1
} NODE;
/*
* A Huffman tree is set up for decoding, not encoding. When encoding,
* I first walk through the tree and build up a table of codes for
* each symbol. The codes are stored in this CODE structure.
*/

typedef struct code {
	unsigned int code;
	int code_bits;
} CODE;

/*
* The special EOS symbol is 256, the first available symbol after all
* of the possible bytes. When decoding, reading this symbol
* indicates that all of the data has been read in.
*/
#define END_OF_STREAM 256

/*
* Local function prototypes, defined with or without ANSI prototypes.
*/
#ifdef __STDC__
void count_bytes( FILE *input, unsigned long *long_counts );
void scale_counts( unsigned long *long_counts, NODE *nodes );
int build_tree( NODE *nodes );
void convert_tree_to_code( NODE *nodes, CODE *codes, unsigned int code_so_far, int bits, int node );
void output_counts( BIT_FILE *output, NODE *nodes );
void input_counts( BIT_FILE *input, NODE *nodes );
void print_model( NODE *nodes, CODE *codes );
void compress_data( FILE *input, BIT_FILE *output, CODE *codes );
void expand_data( BIT_FIle *input, FILE *output, NODE *nodes,
int root_node );
void print_char( int c );
#else /* __STDC__ */

void count_bytes();
void scale_counts();
int build_tree();
void convert_tree_to_code();
void output_counts();
void input_counts();
void print_model();
void compress_data();
void expand_data();
void print_char();

#endif /* __STDC__ */

/*
* These two strings are used by MAIN-C.C and MAIN-E.C to print
* messages of importance to the use of the program.
*/
char *CompressionName = "static order 0 model with Huffman coding";
char *Usage = "infile outfile [-d]\n\n\ Specifying -d will dump the modeling\ data\n";

/*
* CompressFile is the compression routine called by MAIN-C.C. It
* looks for a single additional argument to be passed to it from
* the command line: "-d". If a "-d" is present, it means the
* user wants to see the model data dumped out for debugging
* purposes.
*
* This routine works in a fairly straightforward manner. First,
* it has to allocate storage for three different arrays of data.
* Next, it counts all the bytes in the input file. The counts
* are all stored in long int, so the next step is to scale them down
* to single byte counts in the NODE array. After the counts are
* scaled, the Huffman decoding tree is built on top of the NODE
* array. Another routine walks through the tree to build a table
* of codes, one per symbol. Finally, when the codes are all ready,
* compressing the file is a simple matter. After the file is
* compressed, the storage is freed up, and the routine returns.
*
*/

void CompressFile( input, output, argc, argv )
FILE *input;
BIT_FILE *output;
int argc;
char *argv[];
{
	unsigned long *counts;
	NODE *nodes;
	CODE *codes;
	int root_node;
	
	counts = ( unsigned long *) calloc( 256, sizeof( unsigned long ) );
	if ( counts == NULL )
		fatal_error( "Error allocating counts array\n" );
	if ( ( nodes = (NODE *)	calloc( 514, sizeof( NODE ) ) ) == NULL )
		fatal_error( "Error allocating nodes array\n" );
	if ( ( codes = (CODE *)	calloc( 257, sizeof( CODE ) ) ) == NULL )
		fatal_error( "Error allocating codes array\n" );
	count_bytes( input, counts );
	scale_counts( counts, nodes );
	output_counts( output, nodes );
	root_node = build_tree( nodes );
	convert_tree_to_code( nodes, codes, 0, 0, root_node );
	if ( argc > 0 && strcmp( argv[ 0 ], "-d" ) == 0 )
		print_model( nodes, codes );
	compress_data( input, output, codes );
	free( (char *) counts );
	free( (char *) nodes );
	free( (char *) codes );
}



