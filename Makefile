try: stack.o list.o queue.o mydiff.o mypatch.o stack.h list.h queue.h
	cc stack.o list.o queue.o mydiff.o -o project1
	cc stack.o queue.o mypatch.o -o project2
stack.o: stack.c stack.h
	cc -c -Wall stack.c
queue.o: queue.c queue.h
	cc -c -Wall queue.c
list.o: list.c list.h
	cc -c -Wall list.c
mydiff.o: mydiff.c
	cc -c -Wall mydiff.c
mypatch.o: mypatch.c
	cc -c -Wall mypatch.c 
