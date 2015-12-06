#ifndef __IDC__
#define __IDC__

#include "queue.c"

/**
 *  Author  : K. Sayood
 *  Modified: Djuned Fernando Djusdek - 5112.100.071
 *  Last mod: 12/5/15
 */

struct Queue {
	struct Node* front;
	struct Node* rear;
	
	int size;
};

void write_to_file(FILE **, struct Queue **);

//float ent(float [], int );
int image_size(char [], int *, int *);
void readimage(char [], unsigned char **, int, int);
int encuqi(int, int *, int);
//int decuqi(int, int *);
void stuffit(int , int , struct Queue **, int );
//void stuffit(int , int , FILE **, int );
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

#include "idc.c"

#endif
