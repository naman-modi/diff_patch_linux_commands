typedef struct lnode {
	int nn;
	struct lnode *next, *prev;
}lnode;

typedef struct list {
	struct lnode *head, *tail;
	int lcount;
}list;

void linit(list *l);
void insert(list *l, int num, int pos);
int remov(list *l, int pos);
void ladd(list *l, int num);//ladd and ldelete work together as FIFO functions
int ldelete(list *l);
int listisempty(list *l);
void copylist(list *l1, list *l2);
//void printlist(list *l);
