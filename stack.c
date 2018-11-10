#include <stdlib.h>
#include"stack.h"

void init(stack *s) {
	*s = NULL;
}

int isempty(stack *s) {
	return *s == NULL;
}

int isfull(stack *s) {
	return 0;
}

void push(stack *s, int num) {
	node *temp = (node *) malloc(sizeof(node));
	node *tempnode = *s;
	temp->val = num;
	temp->next = NULL;
	if(!*s)
		*s = temp;
	else {
		while(tempnode->next)
			tempnode = tempnode->next;
		tempnode->next = temp;
	}
}

int pop(stack *s) {
	int ret;
	node *temp = *s;
	if((*s)->next == NULL) {
		ret = (*s)->val;
		free(*s);
		*s = NULL;
	}
	else {
		while(temp->next->next)
			temp = temp->next;
		ret = temp->next->val;
		free(temp->next);
		temp->next = NULL;
	}
	return ret;
}

