CC 			= gcc
CFLAGS 	= -Wall -g
COMMON	= io.o protocol.o datagram.o
HEADER	= ${COMMON.o:.h}

all: client server

client: client.o ${COMMON} ${HEADER}
	${CC} ${CFLAGS} -o client $^

server: server.o ${COMMON} ${HEADER}
	${CC} ${CFLAGS} -o server $^

clean:
	rm -rf client server *.o
