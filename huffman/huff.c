/********************** Start of HUFF.C *************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lib/bitio.h"
#include "../lib/errhand.h"
#include "../lib/main.h"

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
	int child_1;
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
void expand_data( BIT_FILE *input, FILE *output, NODE *nodes, int root_node );
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
char *Usage = "infile outfile [-d]\n\n Specifying -d will dump the modeling data\n";

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

/*
* ExpandFile is the routine called by MAIN-E.C to expand a file that
* has been compressed with order 0 Huffman coding. This routine has
* a simpler job than that of the Compression routine. All it has to
* do is read in the counts that have been stored in the compressed
* file, then build the Huffman tree. The data can then be expanded
* by reading in a bit at a time from the compressed file. Finally,
* the node array is freed and the routine returns.
*
*/
void ExpandFile( input, output, argc, argv ) 
BIT_FILE *input;
FILE *output;
int argc;
char *argv[];
{
	NODE *nodes;
	int root_node;
	if ( ( nodes = (NODE *) calloc( 514, sizeof( NODE ) ) ) == NULL )
		fatal_error( "Error allocating nodes array\n" );
	
	input_counts( input, nodes );
	root_node = build_tree( nodes );
	
	if ( argc > 0 && strcmp( argv[ 0 ], "-d" ) == 0 )
		print_model( nodes, 0 );
	
	expand_data( input, output, nodes, root_node );
	free( (char *) nodes );
}

/*
* In order for the compressor to build the same model, I have to
* store the symbol counts in the compressed file so the expander can
* read them in. In order to save space, I don't save all 256 symbols
* unconditionally. The format used to store counts looks like this:
*
* start, stop, counts, start, stop, counts, ... 0
*
* This means that I store runs of counts, until all the non-zero
* counts have been stored. At this time the list is terminated by
* storing a start value of 0. Note that at least 1 run of counts has
* to be stored, so even if the first start value is 0, I read it in.
* It also means that even in an empty file that has no counts, I have
* to pass at least one count, which will have a value of 0.
*
* In order to efficiently use this format, I have to identify runs of
* non-zero counts. Because of the format used, I don't want to stop a
* run because of just one or two zeros in the count stream. So I have
* to sit in a loop looking for strings of three or more zero values
* in a row.
*
* This is simple in concept, but it ends up being one of the most
* complicated routines in the whole program. A routine that just
* writes out 256 values without attempting to optimize would be much
* simpler, but would hurt compression quite a bit on small files.
*
*/

void output_counts ( output, nodes ) 
BIT_FILE *output;
NODE *nodes;
{
	int first;
	int last;
	int next;
	int i;
	
	first = 0;	
	while ( first < 255 && nodes[ first ].count == 0 )
		first++;
	
	/*
	* Each time I hit the start of the loop, I assume that first is the
	* start of a run of non-zero values. The rest of the loop is
	* concerned with finding the value for last, which is the end of the
	* run, and the value of next, which is the start of the next run.
	* At the end of the loop, I assign next to first, so it starts in on
	* the next run.
	*/
	
	for ( ; first < 256 ; first = next) {
		last = first + 1;
		
		for ( ; ; ) {
			for ( ; last < 256 ; last ++ )
				if ( nodes[ last ].count == 0 )
					break;
			last--;	
	
			for ( next = last + 1; next < 256; next++ )
				if ( nodes[ next ].count != 0 )
					break;
	
			if ( next > 255 )
				break;

			if ( ( next - last ) > 3 )
				break;
			
			last = next;
		};
		/*
		* Here is where I output first, last, and all the counts in between.
		*/
		if ( putc( first, output->file ) != first)
			fatal_error( "Error writing byte counts\n" );
		if ( putc( last, output->file ) != last)
			fatal_error( "Error writing byte counts\n" );
		for ( i = first ; i <= last ; i++ ) {
			if ( putc( nodes[ i ]. count, output->file ) != (int) nodes[ i ]. count)
				fatal_error( "Error writing byte counts\n" );
		}
	}
	if ( putc( 0, output->file ) != 0 )
		fatal_error( "Error writing byte counts\n" );
}

/*
* When expanding, I have to read in the same set of counts. This is
* quite a bit easier that the process of writing them out, since no
* decision making needs to be done. All I do is read in first, check
* to see if I am all done, and if not, read in last and a string of
* counts.
*/
void input_counts( input, nodes) 
BIT_FILE *input;
NODE *nodes;
{	
	int first;
	int last;
	int i;
	int c;
	
	for ( i = 0 ; i < 256 ; i++ )
		nodes[ i ]. count = 0;
	
	if ( ( first = getc( input->file ) ) == EOF)
		fatal_error( "Error reading byte counts\n" );
	if ( ( last = getc( input->file ) ) == EOF )
		fatal_error( "Error reading byte counts\n" );
	
	for ( ; ; ) {
		for ( i = first ; i <= last ; i++ )
			if ( ( c = getc( input->file ) ) == EOF)
				fatal_error( "Error reading byte counts\n" );
			else
				nodes[ i ]. count = (unsigned int) c;

		if ( ( first = getc( input->file ) ) == EOF )
			fatal_error( "Error reading byte counts\n" );
		if ( first == 0)
			break;
		if ( ( last = getc( input->file ) ) == EOF )
			fatal_error( "Error reading byte counts\n" );
	}
	nodes[ END_OF_STREAM ].count = 1;
}

/*
* This routine counts the frequency of occurence of every byte in
* the input file. It marks the place in the input stream where it
* started, counts up all the bytes, then returns to the place where
* it started. In most C implementations, the length of a file
* cannot exceed an unsigned long, so this routine should always
* work.
*/
#ifndef SEEK_SET
#define SEEK_SET 0
#endif

void count_bytes( input, counts) 
FILE *input;
unsigned long *counts;
{	
	long input_marker;
	int c;
	
	input_marker = ftell( input );
	while ( ( c = getc( input ) ) != EOF )
		counts[ c ]++;	
	fseek( input, input_marker, SEEK_SET );
}

/*
* In order to limit the size of my Huffman codes to 16 bits, I scale
* my counts down so they fit in an unsigned char, and then store them
* all as initial weights in my NODE array. The only thing to be
* careful of is to make sure that a node with a non-zero count doesn't
* get scaled down to 0. Nodes with values of 0 don't get codes.
*/
void scale_counts( counts, nodes ) 
unsigned long *counts;
NODE *nodes;
{	
	unsigned long max_count;
	int i;
	
	max_count = 0;
	for ( i = 0 ; i < 256 ; i++ )
		if ( counts[ i ] > max_count )
			max_count = counts[ i ];

	if ( max_count == 0 ) {
		counts[ 0 ] = 1;
		max_count = 1;
	}
	max_count = max_count / 255;
	max_count = max_count + 1;
	
	for ( i = 0 ; i < 256 ; i++ ) {
		nodes[ i ].count = (unsigned int) ( counts[ i ] / max_count );
		if ( nodes[ i ].count == 0 && counts[ i ] !=0 )
			nodes[ i ].count = 1;
	}
	nodes[ END_OF_STREAM ].count = 1;
}

/*
* Building the Huffman tree is fairly simple. All of the active nodes
* are scanned in order to locate the two nodes with the minimum
* weights. These two weights are added together and assigned to a new
* node. The new node makes the two minimum nodes into its 0 child
* and 1 child. The two minimum nodes are then marked as inactive.
* This process repeats until there is only one node left, which is
* the root node. The tree is done, and the root node is passed back
* to the calling routine.
*
* Node 513 is used here to arbitratily provide a node with a guaran
* teed maximum value. It starts off being min_1 and min_2. After all
* active nodes have been scanned, I can tell if there is only one
* active node left by checking to see if min_1 is still 513.
*/
int build_tree( nodes ) 
NODE *nodes;
{	
	int next_free;
	int i;
	int min_1;
	int min_2;
	nodes[ 513 ].count = 0xffff;

	for ( next_free = END_OF_STREAM + 1 ; ; next_free++ ) {
		min_1 = 513;
		min_2 = 513;
		
		for ( i = 0 ; i < next_free; i++ )
			if ( nodes[ i ].count != 0) {
				if ( nodes[ i ].count < nodes[ min_1 ].count ) {
					min_2 = min_1;
					min_1 = i;
				} 
				else if ( nodes[ i ].count < nodes[ min_2 ].count)
					min_2 = i;
			}
		
		if ( min_2 == 513 )
			break;
			
		nodes[ next_free ].count = nodes[ min_1 ].count	+ nodes[ min_2 ].count;
		nodes[ min_1 ].saved_count = nodes[ min_1 ].count;
		nodes[ min_1 ].count = 0;
		nodes[ min_2 ].saved_count = nodes[ min_2 ].count;
		nodes[ min_2 ].count = 0;
		nodes[ next_free ].child_0 = min_1;
		nodes[ next_free ].child_1 = min_2;
	}
	next_free--;
	nodes[ next_free ].saved_count = nodes[ next_free ].count;
	return( next_free );
}

/*
* Since the Huffman tree is built as a decoding tree, there is
* no simple way to get the encoding values for each symbol out of
* it. This routine recursively walks through the tree, adding the
* child bits to each code until it gets to a leaf. When it gets
* to a leaf, it stores the code value in the CODE element, and
* returns.
*/
void convert_tree_to_code( nodes, codes, code_so_far, bits, node ) 
NODE *nodes;
CODE *codes;
unsigned int code_so_far;
int bits;
int node;
{	
	if ( node <= END_OF_STREAM ) {
		codes[ node ].code = code_so_far;
		codes[ node ].code_bits = bits;
		return;
	}
	code_so_far <<= 1;
	bits++;
	convert_tree_to_code( nodes, codes, code_so_far, bits, nodes[ node ]. child_0 );
	convert_tree_to_code( nodes, codes, code_so_far | 1, bits, nodes[ node ].child_1 );
}

/*
* If the -d command line option is specified, this routine is called
* to print out some of the model information after the tree is built.
* Note that this is the only place that the saved_count NODE element
* is used for anything at all, and in this case it is just for
* diagnostic information. By the time I get here, and the tree has
* been built, every active element will have 0 in its count.
*/
void print_model( nodes, codes ) 
NODE *nodes;
CODE *codes;
{	
	int i;
	
	for ( i = 0 ; i < 513 ; i++ ) {
		if ( nodes[ i ].saved_count != 0 ) {
			printf( "node=" );
			print_char( i );
			printf( " count=%3d", nodes[ i ].saved_count );
			printf( " child_0=" );
			print_char( nodes[ i ]. child_0 );
			printf( " child_1=" );
			print_char( nodes[ i ].child_1 );
			
			if ( codes && i <= END_OF_STREAM ) {
				printf( " Huffman code=" );
				FilePrintBinary( stdout, codes[ i ].code, codes[ i ].code_bits );
			}
			printf( "\n" );
		}
	}
}

/*
* The print_model routine uses this function to print out node num
* bers. The catch is if it is a printable character, it gets printed
* out as a character. This makes the debug output a little easier to
* read.
*/
void print_char( c ) 
int c;
{
	if ( c >= 0x20 && c < 127 )
		printf( "'%c'", c );
	else
		printf( "%3d", c );
}

/*
* Once the tree gets built, and the CODE table is built, compressing
* the data is a breeze. Each byte is read in, and its corresponding
* Huffman code is sent out.
*/
void compress_data( input, output, codes ) 
FILE *input;
BIT_FILE *output;
CODE *codes;
{	
	int c;
	while ( ( c = getc( input ) ) != EOF )
		OutputBits( output, (unsigned long) codes[ c ].code, codes[ c ].code_bits );
	OutputBits( output, (unsigned long) codes[ END_OF_STREAM ].code, codes[ END_OF_STREAM ].code_bits );
}

/*
* Expanding compressed data is a little harder than the compression
* phase. As each new symbol is decoded, the tree is traversed,
* starting at the root node, reading a bit in, and taking either the
* child_0 or child_1 path. Eventually, the tree winds down to a
* leaf node, and the corresponding symbol is output. If the symbol
* is the END_OF_STREAM symbol, it doesn't get written out, and
* instead the whole process terminates.
*/

void expand_data( input, output, nodes, root_node ) 
BIT_FILE *input;
FILE *output;
NODE *nodes;
int root_node;
{
	int node;
	for ( ; ; ) {
		node = root_node;
		do {
			if ( InputBit( input ) )
				node = nodes[ node ].child_1;
			else
				node = nodes[ node ].child_0;
		} while ( node > END_OF_STREAM );
		
		if ( node == END_OF_STREAM )
			break;
		if ( ( putc( node, output ) ) != node )
			fatal_error( "Error trying to write byte to output" );
	}
}
/******************************End of HUFF.C***************************/

