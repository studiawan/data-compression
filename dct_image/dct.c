/*************************** Start of DCT.C *****************************
*
* This is the DCT module, which implements a graphics compression
* program based on the discrete cosine transform. It needs to be
* linked with the standard support routines.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bitio.h"
#include "errhand.h"

/*
 * A few parameters that could be adjusted to modify the compression
 * algorithm.  The first two define the number of rows and columns in
 * the grey scale image.   The last one, 'N', defines the DCT block
 * size.
 */
#define ROWS            200
#define COLS            320
#define N               8

/*
 * This macro is used to ensure correct rounding of integer values.
 */
#define ROUND( a )      ( ( (a) < 0 ) ? (int) ( (a) - 0.5 ) : \
                                                  (int) ( (a) + 0.5 ) )

char *CompressionName = "DCT compression";
char *Usage           = "infile outfile [quality]\nQuality from 0-25";

/*
 * Function prototypes for both ANSI and K&R.
 */
#ifdef __STDC__

void Initialize( int quality );
void ReadPixelStrip( FILE *input, unsigned char strip[ N ][ COLS ] );
int InputCode( BIT_FILE *input );
void ReadDCTData( BIT_FILE *input, int input_data[ N ][ N ] );
void OutputCode( BIT_FILE *output_file, int code );
void WriteDCTData( BIT_FILE *output_file, int output_data[ N ][ N ] );
void WritePixelStrip( FILE *output, unsigned char strip[ N ][ COLS ] );
void ForwardDCT( unsigned char *input[ N ], int output[ N ][ N ] );
void InverseDCT( int input[ N ][ N ], unsigned char *output[ N ] );
void CompressFile( FILE *input, BIT_FILE *output, int argc, char *argv[] );
void ExpandFile( BIT_FILE *input, FILE *output, int argc, char *argv[] );

#else

void Initialize();
void ReadPixelStrip();
int InputCode();
void ReadDCTData();
void OutputCode();
void WriteDCTData();
void WritePixelStrip();
void ForwardDCT();
void InverseDCT();
void CompressFile();
void ExpandFile();

#endif

/*
 * Global data used at various places in the program.
 */

unsigned char PixelStrip[ N ][ COLS ];
double C[ N ][ N ];
double Ct[ N ][ N ];
int InputRunLength;
int OutputRunLength;
int Quantum[ N ][ N ];

struct zigzag {
    int row;
    int col;
} ZigZag[ N * N ] =
{
    {0, 0},
    {0, 1}, {1, 0},
    {2, 0}, {1, 1}, {0, 2},
    {0, 3}, {1, 2}, {2, 1}, {3, 0},
    {4, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 4},
    {0, 5}, {1, 4}, {2, 3}, {3, 2}, {4, 1}, {5, 0},
    {6, 0}, {5, 1}, {4, 2}, {3, 3}, {2, 4}, {1, 5}, {0, 6},
    {0, 7}, {1, 6}, {2, 5}, {3, 4}, {4, 3}, {5, 2}, {6, 1}, {7, 0},
    {7, 1}, {6, 2}, {5, 3}, {4, 4}, {3, 5}, {2, 6}, {1, 7},
    {2, 7}, {3, 6}, {4, 5}, {5, 4}, {6, 3}, {7, 2},
    {7, 3}, {6, 4}, {5, 5}, {4, 6}, {3, 7},
    {4, 7}, {5, 6}, {6, 5}, {7, 4},
    {7, 5}, {6, 6}, {5, 7},
    {6, 7}, {7, 6},
    {7, 7}
};

/*
 * The initialization routine has the job of setting up the Cosine
 * Transform matrix, as well as its transposed value.  These two matrices
 * are used when calculating both the DCT and its inverse.  In addition,
 * the quantization matrix is set up based on the quality parameter passed
 * to this routine.  Additionally, the two run length parameters are both
 * set to 0.
 */

void Initialize( quality )
int quality;
{
    int i;
    int j;
    double pi = atan( 1.0 ) * 4.0;

    for ( i = 0 ; i < N ; i++ )
        for ( j = 0 ; j < N ; j++ )
            Quantum[ i ][ j ] = 1 + ( ( 1 + i + j )  * quality );
    OutputRunLength = 0;
    InputRunLength = 0;
    for ( j = 0 ; j < N ; j++ ) {
        C[ 0 ][ j ] = 1.0 / sqrt( (double) N );
        Ct[ j ][ 0 ] = C[ 0 ][ j ];
    }
    for ( i = 1 ; i < N ; i++ ) {
        for ( j = 0 ; j < N ; j++ ) {
            C[ i ][ j ] = sqrt( 2.0 / N ) * cos( pi * ( 2 * j + 1 ) * i / ( 2.0 * N ) );
            Ct[ j ][ i ] = C[ i ][ j ];
        }
    }
}

/*
 * This routine is called when compressing a grey scale file.  It reads
 * in a strip that is N (usually 8) rows deep and COLS (usually 320)
 * columns wide.  This strip is then repeatedly processed, a block at a
 * time, by the forward DCT routine.
 */
void ReadPixelStrip( input, strip )
FILE *input;
unsigned char strip[ N ][ COLS ];
{
    int row;
    int col;
    int c;

    for ( row = 0 ; row < N ; row++ )
        for ( col = 0 ; col < COLS ; col++ ) {
           c = getc( input );
           if ( c == EOF )
               fatal_error( "Error reading input grey scale file" );
           strip[ row ][ col ] = (unsigned char) c;
        }
}
/*
 * This routine reads in a DCT code from the compressed file.  The code
 * consists of two components, a bit count, and an encoded value.  The
 * bit count is encoded as a prefix code with the following binar
 * values:
 *
 *               Number of Bits   Binary Code
 *              --------------   -----------
 *                    0              00
 *                    1              010
 *                    2              011
 *                    3              1000
 *                    4              1001
 *                    5              1010
 *                    6              1011
 *                    7              1100
 *                    8              1101
 *                    9              1110
 *                    10             1111
 *
 * A bit count of zero is followed by a four bit number telling how many
 * zeros are in the encoded run.  A value of 1 through ten indicates a
 * code value follows, which takes up that many bits.  The encoding of values
 * into this system has the following characteristics:
 *
 *         Bit Count               Amplitudes
 *         ---------       --------------------------
 *             1                      -1, 1
 *             2                -3 to -2, 2 to 3
 *             3                -7 to -4, 4 to 7
 *             4               -15 to -8, 8 to 15
 *             5              -31 to -16, 16 to 31
 *             6              -63 to -32, 32 to 64
 *             7             -127 to -64, 64 to 127
 *             8            -255 to -128, 128 to 255
 *             9            -511 to -256, 256 to 511
 *            10           -1023 to -512, 512 to 1023
 *
 */

int InputCode( input_file )
BIT_FILE *input_file;
{
    int bit_count;
    int result;

    if ( InputRunLength > 0 ) {
        InputRunLength--;
        return( 0 );
    }
    bit_count = (int) InputBits( input_file, 2 );
    if ( bit_count == 0 ) {
        InputRunLength = (int) InputBits( input_file, 4 );
        return( 0 );
    }
    if ( bit_count == 1 )
        bit_count = (int) InputBits( input_file, 1 ) + 1;
    else
        bit_count = (int) InputBits( input_file, 2 ) + ( bit_count << 2 ) - 5;
    result = (int) InputBits( input_file, bit_count );
    if ( result & ( 1 << ( bit_count - 1 ) ) )
        return( result );
    return( result - ( 1 << bit_count ) + 1 );
}

/*
 * This routine reads in a block of encoded DCT data from a compressed file.
 * The routine reorders it in row major format, and dequantizes it using
 * the quantization matrix.
 */

void ReadDCTData( input_file, input_data )
BIT_FILE *input_file;
int input_data[ N ][ N ];
{
    int i;
    int row;
    int col;

    for ( i = 0 ; i < ( N * N ) ; i++ ) {
        row = ZigZag[ i ].row;
        col = ZigZag[ i ].col;
        input_data[ row ][ col ] = InputCode( input_file ) *
                                   Quantum[ row ][ col ];
    }
}

/*
 * This routine outputs a code to the compressed DCT file.  For specs
 * on the exact format, see the comments that go with InputCode, shown
 * earlier in this file.
 */

void OutputCode( output_file, code )
BIT_FILE *output_file;
int code;
{
    int top_of_range;
    int abs_code;
    int bit_count;

    if ( code == 0 ) {
        OutputRunLength++;
        return;
    }
    if ( OutputRunLength != 0 ) {
        while ( OutputRunLength > 0 ) {
            OutputBits( output_file, 0L, 2 );
            if ( OutputRunLength <= 16 ) {
                OutputBits( output_file,
                            (unsigned long) ( OutputRunLength - 1 ), 4 );
                OutputRunLength = 0;
            } else {
                OutputBits( output_file, 15L, 4 );
                OutputRunLength -= 16;
            }
        }
    }
    if ( code < 0 )
        abs_code = -code;
    else
        abs_code = code;
    top_of_range = 1;
    bit_count = 1;
    while ( abs_code > top_of_range ) {
        bit_count++;
        top_of_range = ( ( top_of_range + 1 ) * 2 ) - 1;
    }
    if ( bit_count < 3 )
        OutputBits( output_file, (unsigned long) ( bit_count + 1 ), 3 );
    else
        OutputBits( output_file, (unsigned long) ( bit_count + 5 ), 4 );
    if ( code > 0 )
        OutputBits( output_file, (unsigned long) code, bit_count );
    else
        OutputBits( output_file, (unsigned long) ( code + top_of_range ),
                    bit_count );
}

/*
 * This routine takes DCT data, puts it in Zig Zag order, the quantizes
 * it, and outputs the code.
 */

void WriteDCTData( output_file, output_data )
BIT_FILE *output_file;
int output_data[ N ][ N ];
{
    int i;
    int row;
    int col;
    double result;

    for ( i = 0 ; i < ( N * N ) ; i++ ) {
        row = ZigZag[ i ].row;
        col = ZigZag[ i ].col;
        result = output_data[ row ][ col ] / Quantum[ row ][ col ];
        OutputCode( output_file, ROUND( result ) );
    }
}

/*
 * This routine writes out a strip of pixel data to a GS format file.
 */

void WritePixelStrip( output, strip )
FILE *output;
unsigned char strip[ N ][ COLS ];
{
    int row;
    int col;

    for ( row = 0 ; row < N ; row++ )
        for ( col = 0 ; col < COLS ; col++ )
           putc( strip[ row ][ col ], output );
}

/*
 * The Forward DCT routine implements the matrix function:
 *
 *                     DCT = C * pixels * Ct
 */

void ForwardDCT( input, output )
unsigned char *input[ N ];
int output[ N ][ N ];
{
    double temp[ N ][ N ];
    double temp1;
    int i;
    int j;
    int k;

/*  MatrixMultiply( temp, input, Ct ); */
    for ( i = 0 ; i < N ; i++ ) {
        for ( j = 0 ; j < N ; j++ ) {
            temp[ i ][ j ] = 0.0;
            for ( k = 0 ; k < N ; k++ )
                 temp[ i ][ j ] += ( (int) input[ i ][ k ] - 128 ) *
                                   Ct[ k ][ j ];
        }
    }

/*  MatrixMultiply( output, C, temp ); */
    for ( i = 0 ; i < N ; i++ ) {
        for ( j = 0 ; j < N ; j++ ) {
            temp1 = 0.0;
            for ( k = 0 ; k < N ; k++ )
                temp1 += C[ i ][ k ] * temp[ k ][ j ];
            output[ i ][ j ] = ROUND( temp1 );
        }
    }
}

/*
 * The Inverse DCT routine implements the matrix function:
 *
 *                     pixels = C * DCT * Ct
 */

void InverseDCT( input, output )
int input[ N ][ N ];
unsigned char *output[ N ];
{
    double temp[ N ][ N ];
    double temp1;
    int i;
    int j;
    int k;

/*  MatrixMultiply( temp, input, C ); */
    for ( i = 0 ; i < N ; i++ ) {
        for ( j = 0 ; j < N ; j++ ) {
            temp[ i ][ j ] = 0.0;
            for ( k = 0 ; k < N ; k++ )
                temp[ i ][ j ] += input[ i ][ k ] * C[ k ][ j ];
        }
    }

/*  MatrixMultiply( output, Ct, temp ); */
    for ( i = 0 ; i < N ; i++ ) {
        for ( j = 0 ; j < N ; j++ ) {
            temp1 = 0.0;
            for ( k = 0 ; k < N ; k++ )
                temp1 += Ct[ i ][ k ] * temp[ k ][ j ];
            temp1 += 128.0;
            if ( temp1 < 0 )
                output[ i ][ j ] = 0;
            else if ( temp1 > 255 )
                output[ i ][ j ] = 255;
            else
                output[ i ][ j ] = (unsigned char) ROUND( temp1 );
        }
    }
}

/*
 * This is the main compression routine.  By the time it gets called,
 * the input and output files have been properly opened, so all it has to
 * do is the compression.  Note that the compression routine expects an
 * additional parameter, the quality value, ranging from 0 to 25.
 */

void CompressFile( input, output, argc, argv )
FILE *input;
BIT_FILE *output;
int argc;
char *argv[];
{
    int row;
    int col;
    int i;
    unsigned char *input_array[ N ];
    int output_array[ N ][ N ];
    int quality;

    if ( argc-- > 0 )
        quality = atoi( argv[ 0 ] );
    else
        quality = 3;
    if ( quality < 0 || quality > 50 )
        fatal_error( "Illegal quality factor of %d\n", quality );
    printf( "Using quality factor of %d\n", quality );
    Initialize( quality );
    OutputBits( output, (unsigned long) quality, 8 );
    for ( row = 0 ; row < ROWS ; row += N ) {
        ReadPixelStrip( input, PixelStrip );
        for ( col = 0 ; col < COLS ; col += N ) {
            for ( i = 0 ; i < N ; i++ )
                input_array[ i ] = PixelStrip[ i ] + col;
            ForwardDCT( input_array, output_array );
            WriteDCTData( output, output_array );
        }
    }
    OutputCode( output, 1 );
    while ( argc-- > 0 )
        printf( "Unused argument: %s\n", *argv++ );
}

/*
 * The expansion routine reads in the compressed data from the DCT file,
 * then writes out the decompressed grey scale file.
 */

void ExpandFile( input, output, argc, argv )
BIT_FILE *input;
FILE *output;
int argc;
char *argv[];
{
    int row;
    int col;
    int i;
    int input_array[ N ][ N ];
    unsigned char *output_array[ N ];
    int quality;

    quality = (int) InputBits( input, 8 );
    printf( "\rUsing quality factor of %d\n", quality );
    Initialize( quality );
    for ( row = 0 ; row < ROWS ; row += N ) {
        for ( col = 0 ; col < COLS ; col += N ) {
            for ( i = 0 ; i < N ; i++ )
                output_array[ i ] = PixelStrip[ i ] + col;
            ReadDCTData( input, input_array );
            InverseDCT( input_array, output_array );
        }
        WritePixelStrip( output, PixelStrip );
    }
    while ( argc-- > 0 )
        printf( "Unused argument: %s\n", *argv++ );
}

/************************** End of DCT.C *************************/
