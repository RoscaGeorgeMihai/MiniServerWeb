main: main.o threadpool.o queue.o clientshandler.o start_stop.o
	gcc -g main.o threadpool.o queue.o clientshandler.o start_stop.o -o main -lpthread

main.o: main.c threadpool.h start_stop.h 
	gcc -g -c main.c -o main.o

threadpool.o: threadpool.c threadpool.h queue.h
	gcc -g -c threadpool.c -o threadpool.o

queue.o: queue.c queue.h
	gcc -g -c queue.c -o queue.o

clientshandler.o: clientshandler.c clientshandler.h
	gcc -g -c clientshandler.c -o clientshandler.o

start_stop.o: start_stop.c start_stop.h
	gcc -g -c start_stop.c -o start_stop.o

clean:
	rm -f *.o main server
