//#include "idc.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void write_to_file(FILE **ofp, struct Queue **code_write) {
	unsigned char temp[1];
	temp[0] = temp[0] & 0x00;
	for (int i=0; i<8; i++) {
		if (Front(&(*code_write)->front) == '1') {
			temp[0] ^= 0x01;
		}
		
		if (i != 7) {
			temp[0] <<= 1;
		}
		
		Dequeue(&(*code_write)->front, &(*code_write)->rear);
		(*code_write)->size--;
	}
	fwrite((char*)temp, 1, sizeof(char), *ofp);
	
	return;
}

//float ent(float [], int );
int image_size(char input_image[], int *row_size, int *col_size) {
	if (strstr(input_image, ".img") != NULL) {
		*row_size = 256;
		*col_size = 256;
		
		return 256 * 256;
	} else if (strstr(input_image, ".y") != NULL) {
		*row_size = 486;
		*col_size = 720;
		
		return 720 * 486;
	} else {
		return 0;
	}
}
void readimage(char input_image[], unsigned char **imgin, int row_size, int col_size) {
	FILE *fp;
	
	fp = fopen(input_image, "rb");
//	printf("%s", input_image);
	
	int row = 0;
	int col;
	char buffer[2];
	buffer[1] = '\0';
	
//	printf("%d %d\n", row_size, col_size);
	
	for (row; row<row_size; ++row) {
		for (col=0; col<col_size; ++col) {
			buffer[0] = 0x00;
			fread(buffer, sizeof(char), 1, fp);
//			printf("%c ", buffer[0]);
			
			imgin[row][col] = buffer[0];
		}
	}
	
	return;
}
int encuqi(int input, int *bound, int numlev) {
	int iter = 1;
	
	for (iter; iter<=numlev; ++iter) {
		if (input < bound[iter]) {
			return input / (bound[iter] - bound[iter-1]);
		}
	}
}
//int decuqi(int, int *);
void stuffit(int lable, int numbits, struct Queue **code_write, int end_flag) {
	unsigned char buffer;
	buffer = (unsigned char) lable;
//	printf("%d", buffer);
	
//	printf("%d ", &*ofp);
	
	int iter = 0;
	buffer <<= (8-numbits);
	for (iter; iter<numbits; ++iter) {
//		printf("%d\n", buffer);
		if ((buffer & 0x80) == 0x80) {
			Enqueue(&(*code_write)->front, &(*code_write)->rear, '1');
			(*code_write)->size++;
		} else {
			Enqueue(&(*code_write)->front, &(*code_write)->rear, '0');
			(*code_write)->size++;
		}
		buffer <<= 1;
//		Print(&(*code_write)->front);
	}
	
//	printf("%d", buffer);
	
	if (end_flag == 1) {
		// write EOF ?
	}
}
//void stuffit(int lable, int numbits, FILE **ofp, int end_flag) {
//	unsigned char buffer[2];
//	buffer[0] = (unsigned char) lable;
////	printf("%d", buffer);
//	
////	printf("%d ", &*ofp);
//	
//	fwrite((char*)buffer, 1, sizeof(char), *ofp);
//	
////	printf("%d", buffer);
//	
//	if (end_flag == 1) {
//		// write EOF ?
//	}
//}
//int readnbits(int , FILE *);
//int uquan_enc(float ,  float , int , float );
//float uquan_dec(int,  float, float );
//int nuquan_enc(float ,  float [], int);
//float nuquan_dec(int,  float [] );
//extern float rangen(int);
//int vqencode( int *, int **,  int, int,float *);
///* vqencode(input,codebook,codebook_size,dimension,&distortion)
//returns the index in the codebook of the closest codeword */
//void unstuff(int , FILE *, int *, int *);
//int readau(char filename[], short aufile[]);
//int get_file_size(char [], int *);
//float predictor(int ,int ,float [],float [], float *);
//
//
//void getim(char *fname, unsigned char *array, int size);
//void storeim(char *fname, unsigned char *array, int size);
//void sort(float *prob, int *loc,int num);
//void huff(float prob[], int loc[], int num, unsigned int *code, char *length);
//void diff(int *diffs, unsigned char *image, int rows, int cols,int num, unsigned char *dimage);
//void getcode(FILE *fp, int num, unsigned int *code, char *length);
//void value(int *values, unsigned char *image, int size,int num);
//void images(int rows, int cols, int *code,char *length, unsigned char *file, unsigned char *dimage);
//int files(int size,int *code,char *length,unsigned char *file);
//
///* define the structure node */
//
//        struct node {
//                float pro; /* probabilities */
//                int l; /* location of probability before sorted */
//                unsigned int code; /* code */
//                struct node *left; /* pointer for binary tree */
//                struct node *right; /* pointer for binary tree */
//                struct node *forward;
//                struct node *back;
//                struct node *parent; /* pointer to parent */
//                int check;
//        };
//
///* define subroutines and pointers to nodes */
//
//typedef struct node NODE;
//typedef struct NODE *BTREE;
//BTREE create_list(float prob[], int loc[], int num);
//void create_code(NODE *root, int lgth, unsigned int *code, char *length);
//
//BTREE make_list (int num);
//void write_code(NODE *root, int lgth, unsigned int *code, char *length, int *loc);
//
//   
//
//
//#define     myabs(x) ((x)<0? -(x): (x))
