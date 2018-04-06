CC 			= gcc
CFLAGS 	= -Wall -g ${shell pkg-config --cflags glib-2.0}
LFLAGS	= -g ${shell pkg-config --libs glib-2.0}
COMMON	= io.o protocol.o datagram.o
HEADER	= ${COMMON.o:.h}

server: server.o ${COMMON} ${HEADER}
	${CC} ${LFLAGS} -o server $^

clean:
	rm -rf client server *.o
