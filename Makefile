CC = gcc
CFLAGS = -Wall

all: client server

client: client.o misc.o
	${CC} ${CFLAGS} -o client $^

server: server.o misc.o
	${CC} ${CFLAGS} -o server $^

clean:
	rm -rf client server *.o
