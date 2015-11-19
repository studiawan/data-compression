#include "idc.h"

void value(int *values, unsigned char *image, int size,int num)
{
	int i;
	for(i = 0; i < size; i++)
    {
		values[image[i]]++;
	}
	return;
}

void getcode(FILE *fp, int num, unsigned int *code, char *length)
{
	return;
}

void sort(float *prob, int *loc,int num)
{
	int i,j;
	//selection sort descending
	for(i = 0; i < num; i++)
    {
		loc[i] = i;
	}

	for(i = 0; i < num-1; i++)
    {
		int temp = i;
		for(j = i+1; j < num; j++)
		{
			if(prob[loc[temp]] < prob[loc[j]])
			{
				temp = j;
			}
			else if(prob[loc[temp]] == prob[loc[j]] && loc[j] < loc[temp])
			{
				temp = j;
			}
		}
		int tmp = loc[i];
		loc[i] = loc[temp];
		loc[temp] = tmp;
	}
	return;
}

void huff(float prob[], int loc[], int num, unsigned int *code, char *length)
{
	return;
}

int files(int size,int *code,char *length,unsigned char *file)
{
	return;
}

