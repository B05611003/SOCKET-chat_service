CC = gcc
DEBUG = -g
CFLAGS = -Wall -pthread -c $(DEBUG)
LFLAGS = -Wall -pthread $(DEBUG)

all:  server client move

server: server.o finalproject.o
	$(CC) $(LFLAGS) server.o finalproject.o -o server


client: client.o finalproject.o
	$(CC) $(LFLAGS) client.o finalproject.o -o client

move:
	mkdir ./test1
	mkdir ./test2
	cp client ./test1
	cp client ./test2

client.o: client.c finalproject.h
	$(CC) $(CFLAGS) client.c

server.o: server.c finalproject.h
	$(CC) $(CFLAGS) server.c

finalproject.o: finalproject.h finalproject.c
	$(CC) $(CFLAGS) finalproject.c

clean:
	rm -rf *.o *~ client server
