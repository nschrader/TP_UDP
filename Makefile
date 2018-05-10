CC 			= gcc
CFLAGS 	= -Wall -std=gnu99 -g ${shell pkg-config --cflags glib-2.0}
LFLAGS	= -g ${shell pkg-config --libs glib-2.0}
COMMON	= io.o protocol.o datagram.o
HEADER	= ${COMMON.o:.h}

server: server.o ${COMMON} ${HEADER}
	${CC} -o server $^ ${LFLAGS}

clean:
	rm -rf client server *.o
