CC=gcc
CFLAGS=-lrt -lpthread -g

all:	reader writer

writer:	writer.c
	$(CC) $(CFLAGS) writer.c -o writer

reader:	reader.c
	$(CC) $(CFLAGS) reader.c -o reader

clean:
	rm writer reader
