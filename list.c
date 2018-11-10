#include<stdio.h>
#include<string.h>
#include<strings.h>
#include<stdlib.h>
#include"list.h"

void linit(list *l) {
	l->head = NULL;
	l->tail = NULL;
	l->lcount = 0;
}

int listisempty(list *l) {
	return l->lcount == 0;
}

void copylist(list *l1, list *l2) {
	lnode *temp;
	int t;
	temp = l1->head;
	while(temp != NULL) {
		t = temp->nn;
		insert(l2, t, l2->lcount);
		temp = temp->next;
	}	
}

void insert(list *l, int num, int pos) {
	lnode *p = l->head;
	int count = 0, j;
	lnode *temp;
	while(p != NULL) { 
		p = p->next;
		count++;
	}
	j = count;
	if(pos > count) {
		return;
	}
	l->lcount++;
	temp = (lnode *)malloc(sizeof(lnode));
	temp->nn = num;
	p = l->head;
	if((pos == 0) && (count == 0)) {
		l->head = temp;
		l->tail = temp;
		temp->next = NULL;
		temp->prev = NULL;
		return;
	}
	if(pos == 0) {
		temp->next = p;
		l->head = temp;
		p->prev = temp;
		temp->prev = NULL;
		return;
	}
	count = 0;
	while((pos - 1) != count) {
		p = p->next;
		count++;
	}
	if((pos)  == j) {
		p->next = temp;
		temp->next = NULL;
		temp->prev = l->tail;
		l->tail = temp;
		return;
	}
	temp->next = p->next;
	temp->prev = p;
	p->next = temp;
	temp->next->prev = temp;
}


int remov(list *l, int pos) {
	lnode *p = l->head, *p1 = l->head;
	int count = 0, ss;
	while(p != NULL) { 
		p = p->next;
		count++;
	}
	if(pos >= count) {
		return -1;
	}
	l->lcount--;
	p = l->head;
	if((p == l->head) && (p == l->tail)) {
		ss = p->nn;
		free(p);
		l->head = NULL;
		l->tail = NULL;
		return ss;
	}
	count = 0;
	while(count != pos) {
		p = p->next;
		count++;
		if(count != pos)
			p1 = p1->next;
	}
	if(p == l->head) {
		ss = p->nn;
		l->head = l->head->next;
		l->head->prev = NULL;
		free(p);
		return ss;
	}
	if(p == l->tail) {
		ss = p->nn;
		l->tail = p1;
		l->tail->next = NULL;
		free(p);
		return ss;
	} 
	ss = p->nn;
	p1->next = p->next;
	p->next->prev = p1;
	free(p);
	return ss;
}

void ladd(list *l, int num) {
	insert(l, num, 0); 
}

int ldelete(list *l) {
	int i, j;
	i = l->lcount - 1;
	j = remov(l, i);
	return j;
}
