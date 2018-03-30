CC 			= gcc
CFLAGS 	= -Wall -g -D_GNU_SOURCE ${shell pkg-config --cflags glib-2.0}
LFLAGS	= -g ${shell pkg-config --libs glib-2.0}
COMMON	= io.o protocol.o datagram.o con_status.o
HEADER	= ${COMMON.o:.h}

all: client server

client: client.o ${COMMON} ${HEADER}
	${CC} ${LFLAGS} -o client $^

server: server.o ${COMMON} ${HEADER}
	${CC} ${LFLAGS} -o server $^

clean:
	rm -rf client server *.o
