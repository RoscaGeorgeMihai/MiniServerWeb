main: main.o threadpool.o queue.o clientshandler.o
	gcc -g main.o threadpool.o queue.o clientshandler.o -o main

main.o: main.c threadpool.h
	gcc -g -c main.c -o main.o

threadpool.o: threadpool.c threadpool.h queue.h
	gcc -g -c threadpool.c -o threadpool.o

queue.o: queue.c queue.h
	gcc -g -c queue.c -o queue.o

clientshandler.o:
	gcc -g -c clientshandler.c -o clientshandler.o

clean:
	rm -f *.o main
