#ifndef QUEUE
#define QUEUE

#include <stdio.h>
#include <stdlib.h>

/**
 *  Author  : Djuned Fernando Djusdek - 5112.100.071
 *  Last mod: 12/5/15
 */

struct Node {
	char data;
	struct Node* next;
};

void Enqueue(struct Node **front, struct Node **rear, char x) {
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	temp->data =x; 
	temp->next = NULL;
	if(*front == NULL && *rear == NULL){
		*front = *rear = temp;
		return;
	}
	(*rear)->next = temp;
	*rear = temp;
}

void Dequeue(struct Node **front, struct Node **rear) {
	struct Node* temp = *front;
	if(front == NULL) {
		printf("Queue is Empty\n");
		return;
	}
	if(*front == *rear) {
		*front = *rear = NULL;
	}
	else {
		*front = (*front)->next;
	}
	free(temp);
}

char Front(struct Node **front) {
	if(*front == NULL) {
		printf("Queue is empty\n");
		return '\0';
	}
	return (*front)->data;
}

void Print(struct Node **front) {
	struct Node* temp = *front;
	printf("write_code:= { ");
	while(temp != NULL) {
		printf("%c, ", temp->data);
		temp = temp->next;
	}
	printf("}\n");
}

#endif
