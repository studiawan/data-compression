#include "idc.h"
#include <stdlib.h>

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
    int i;
    int sentinel = num;
    NODE *head = create_list(prob, loc, num);
    while (head->forward) {
        NODE *rightNode = head;
        NODE *leftNode = head->forward;
        NODE *newNode = (NODE *) malloc(sizeof(NODE));
        newNode->parent = newNode->back = newNode->forward = NULL;
        newNode->left = leftNode;
        newNode->right = rightNode;
        newNode->left->parent = newNode;
        newNode->right->parent = newNode;
        newNode->pro = newNode->left->pro + newNode->right->pro;

        head = head->forward;
        if (head) head = head->forward;
        if (head) head->back = NULL;
        rightNode->forward = rightNode->back = NULL;
        leftNode->forward = leftNode->back = NULL;

        NODE *temp = head, *prev = NULL;
        while (temp && temp->pro <= newNode->pro) {
            prev = temp;
            temp = temp->forward;
        }
        newNode->forward = temp;
        newNode->back = prev;
        if (temp) temp->back = newNode;
        if (prev) prev->forward = newNode;

        if (newNode->forward == head) head = newNode;
    }
    return;
}

NODE* create_list(float prob[], int loc[], int num) {
    int i;
    BTREE firstNode = (NODE *)malloc(sizeof(NODE));
    firstNode->code = loc[num-1];
    firstNode->pro = prob[loc[num-1]];
    firstNode->left = firstNode->right = firstNode->parent = firstNode->forward = firstNode->back = NULL;
    firstNode->check = 0;
    BTREE head = firstNode;
    BTREE tail = firstNode;
    for(i=num-2; i>0; i--) {
        BTREE newNode = (BTREE)malloc(sizeof(NODE));
        newNode->code = loc[i];
        newNode->pro = prob[loc[i]];
        newNode->left = newNode->right = newNode->parent = newNode->forward = newNode->back = NULL;
        newNode->check = 0;
        tail->forward = newNode;
        newNode->back = tail;
        newNode->forward = NULL;
        tail = newNode;
    }
    BTREE lastNode = (BTREE)malloc(sizeof(NODE));
    lastNode->code = loc[0];
    lastNode->pro = prob[loc[0]];
    lastNode->left = lastNode->right = lastNode->parent = lastNode->forward = lastNode->back = NULL;
    lastNode->check = 0;
    tail->forward = lastNode;
    lastNode->back = tail;
    lastNode->forward = NULL;
    return head;
}

int files(int size,int *code,char *length,unsigned char *file)
{
//	return;
}

