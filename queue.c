#include<stdlib.h>
#include"queue.h"

void qinit(queue *q) {
	q->head = q->tail = NULL;
	q->c = 0;
}

void enqueue(queue *q, int n) {
	qnode *temp;
	temp = (qnode *)malloc(sizeof(qnode));
	temp->number = n;
	temp->next = q->head;
	temp->prev = NULL;
	q->c++;
	if(q->tail == NULL) 
		q->tail = temp;
	else
		q->head->prev = temp;
	q->head = temp;
}

int dequeue(queue *q) {
	int nn;
	qnode *temp2;
	q->c--;
	nn = q->tail->number;
	temp2 = q->tail;
	if(q->tail == q->head) {
		q->tail = NULL;
		q->head = NULL;
	} else {
		q->tail = q->tail->prev;
		q->tail->next = NULL;
	}
	free(temp2);
	return nn;
}

int qisfull(queue *q) {
	return 0;
}

int qisempty(queue *q) {
	return q->c == 0;
}
