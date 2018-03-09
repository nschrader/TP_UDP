CC = gcc
CFLAGS = -Wall

all: client server

client: client.o misc.o misc.h
	${CC} ${CFLAGS} -o client $^

server: server.o misc.o misc.h
	${CC} ${CFLAGS} -o server $^

clean:
	rm -rf client server *.o
