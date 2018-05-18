CC 			= gcc
CFLAGS 	= -std=gnu99 ${shell pkg-config --cflags glib-2.0}
LFLAGS	= -g ${shell pkg-config --libs glib-2.0}
COMMON	= io.o protocol.o datagram.o server.o
HEADER	= ${COMMON.o:.h}

debug: CFLAGS += -Wall -g
debug: ${COMMON} ${HEADER}
	${CC} -o server $^ ${LFLAGS}

release: CFLAGS += -DNDEBUG
release: server.o ${COMMON} ${HEADER}
	${CC} -o server $^ ${LFLAGS}

clean:
	rm -rf server *.o
