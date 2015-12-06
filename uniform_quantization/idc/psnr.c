/**
 * For counting PSNR value from ...
 * Lossy Image Compression using Adaptive lsb, Vitter (Adaptive Huffman) & Welch (LZW with Fredkin (Trie Memory))
 * -- OR --
 * https://github.com/santensuru/adaptive-huffman-image-lossy-with-lzw
 *
 * Author: Djuned Fernando Djusdek
 *         5112.100.071
 *         Informatics - ITS
 *
 * Modified by : A. Heynoum Dala Rifat
 * Date : 06/12/2015
 *
 * This code only use for grayscale image from 8-bit *.pgm format file
 */


#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <cstream>
//#include ""Queue.c""

#define SYMBOL 256

int read_from_file_instantly(FILE *file, unsigned char *symbol)
{
	char temp;
	char a = fgetc(file);
	if(a==temp)
	{
		symbol[0]=temp;
		
		return 1;
	}
	else
	{
		return -1;
	}
	
}

double mean_squared_error(FILE *file, FILE *file2, int size)
{
	unsigned char symbol[1];
	unsigned char symbol2[1];
	
	symbol[0] = 0x00;
	symbol2[0] = 0x00;
	
	long long int current = 0;
	int iter;
	
	for(iter=0; iter<size; ++iter)
	{
		read_from_file_instantly(&*file, symbol);
		read_from_file_instantly(&*file2, symbol2);
		
		current += ( (int)symbol[0] - (int)symbol2[0]) * ( (int)symbol[0] - (int)symbol2[0]);
	}
	return 1.0/size*current;
}


double peak_signal_to_noise_ratio( double mse ) {
	return 10.0 * log10( ( SYMBOL - 1 ) * ( SYMBOL - 1 ) / mse );
}

int main( int argc, char* argv[] ) {
	/** argv = {
	 *      0 := this program (.exe)
	 *      1 := filname 1 (.pgm) original
	 *      2 := filname 2 (.pgm / .restore / .restorelzw / .restorelzwo) restore
	 *  }
	 */
	
	if ( argc != 3 ) {
		printf( "\aUsage: %s <filename original> <filename restore>\n", argv[0] );
		
		return 1;
		
	} else {
		/*FILE *in;
		fopen(in, "rb");
		
		FILE *in2;
		in2 = fopen(in2, "rb");
		/*FILE in;
		fopen();
		std::ifstream in;
		in.open( argv[1], std::ios::in | std::ios::binary );
		
		std::ifstream in2;
		in2.open( argv[2], std::ios::in | std::ios::binary );
		*/
		char temp[128];
		
		int width, height;
		int i = 0;
		/*
		do {
			in.getline( temp, 128 );
			if ( i == 1 ) {
				if ( strstr(temp, " ") != NULL ) {
					sscanf( temp, "%d %d", &width, &height );
				} else {
					sscanf( temp, "%d", &width );
					in.getline( temp, 128 );
					sscanf( temp, "%d", &height );
				}
			}
			
			if ( temp[0] == '#' )
				continue;
			else
				i++;
			
		} while( i < 3 );
		
		i=0;
		do {
			in2.getline( temp, 128 );
			if ( temp[0] == '#' )
				continue;
			else
				i++;
			
		} while( i < 3 );
		*/
		int size = width * height;
		
		double mse = mean_squared_error( &in, &in2, size );
		
		printf("MSE := ", mse);
		
		double psnr = peak_signal_to_noise_ratio( mse );
		
		printf("PSNR := ", psnr);
	}
	
	return 0;
}
