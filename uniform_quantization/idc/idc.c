#ifndef _IDC_
#define _IDC_

#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
 *  Author  : Djuned Fernando Djusdek - 5112.100.071
 *  Last mod: 12/6/15                               
 */

void write_to_file(FILE **ofp, struct Queue **code_write) {
	unsigned char temp[1];
	temp[0] &= 0x00;
	int i = 0;
	
	for (i; i<8; ++i) {
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
double get_ratio(FILE **fp, FILE **ofp) {
	int size_ori, size_com;
	fseek(*fp, 0L, SEEK_END);
	size_ori = ftell(*fp);
	
	fseek(*ofp, 0L, SEEK_END);
	size_com = ftell(*ofp);
	
	return size_com * 100.0 / size_ori;
}
int read_from_file(FILE **ifp, struct Queue **code_read) {
	char temp[1];
	temp[0] &= 0x00;
	
	if (fread(temp, sizeof(char), 1, *ifp) > 0) {
		for (int i=0; i<8; i++) {
			if ((temp[0] & 0x80) == 0x80) {
				Enqueue(&(*code_read)->front, &(*code_read)->rear, '1');
				(*code_read)->size++;
				
			} else {
				Enqueue(&(*code_read)->front, &(*code_read)->rear, '0');
				(*code_read)->size++;
				
			}
			temp[0] <<= 1;
		}
		
		return 1;
		
	} else {
		return -1;
		
	}
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
			
			imgin[row][col] = (unsigned char)buffer[0];
		}
	}
	
	fclose(fp);
	
	return;
}
int encuqi(int input, int *bound, int numlev) {
	int iter = 1;
	
	for (iter; iter<=numlev; ++iter) {
		if (input < bound[iter]) {
//			printf("%d %d\n", input, input / (bound[iter] - bound[iter-1]));
			return input / (bound[iter] - bound[iter-1]);
		}
	}
}
int decuqi(int label, int *reco) {
//	printf("%d %d\n", label, reco[label]);
	return reco[label];
}
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
void unstuff(int numbits, struct Queue **code_read, int *buffer, int *count) {
	unsigned char temp[1];
	int i;
	int iter = 0;
	
	while ((*code_read)->size > 0) {
		temp[0] &= 0x00;
		for (i=0; i<numbits; ++i) {
			if (Front(&(*code_read)->front) == '1') {
				temp[0] ^= 0x01;
			}
			
			if (i != numbits-1) {
				temp[0] <<= 1;
			}
			
//			printf("%d ", temp[0]);
			
			Dequeue(&(*code_read)->front, &(*code_read)->rear);
			(*code_read)->size--;
		}
//		printf("%d\n", temp[0]);
		buffer[iter++] = (int) temp[0];
		
//		printf("%d\n",(*code_read)->size);
	}
//	printf("%d", iter);
	*count = iter;
	
	return;
}
//void unstuff(int , FILE **, int *, int *);
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

#endif
