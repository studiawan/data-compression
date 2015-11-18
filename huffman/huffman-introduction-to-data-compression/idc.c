#include "idc.h"

void value(int *values, unsigned char *image, int size,int num){
	int i;
	for(i=0;i<size;i++){
		values[image[i]]++;
	}
}

void getcode(FILE *fp, int num, unsigned int *code, char *length){
	return;
}

void sort(float *prob, int *loc,int num){
	return;
}

void huff(float prob[], int loc[], int num, unsigned int *code, char *length){
	return;
}

int files(int size,int *code,char *length,unsigned char *file){
	return;
}

