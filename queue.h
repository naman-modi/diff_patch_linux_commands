typedef struct qnode {
	int number;
	struct qnode *next;
	struct qnode *prev;
}qnode;

typedef struct queue {
	qnode *head;
	qnode *tail;
	int c;
}queue;

void qinit(queue *q);
void enqueue(queue *q, int n);
int dequeue(queue *q);
int qisfull(queue *q);
int qisempty(queue *q);
