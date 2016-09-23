CC=gcc
CFLAGS=-lrt -lpthread -g

all:	reader writer

writer:	writer.c
	$(CC) $(CFLAGS)  shm.o writer.c -o writer

reader:	reader.c
	$(CC) $(CFLAGS)  shm.o reader.c -o reader


writer.c: shm.o

reader.c: shm.o

shm.o: shm.c
	$(CC) $(CFLAGS) -c shm.c
clean:
	rm writer reader *.o
