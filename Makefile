CC 			= gcc
CFLAGS 	= -std=gnu99 ${shell pkg-config --cflags glib-2.0}
LFLAGS	= -g ${shell pkg-config --libs glib-2.0}
COMMON	= io.o protocol.o datagram.o server.o window.o
HEADER	= ${COMMON.o:.h}

prs: debug
	cp server serveur1-SchraVall
	cp server serveur2-SchraVall
	cp server serveur3-SchraVall
	strip -s serveur1-SchraVall
	strip -s serveur2-SchraVall
	strip -s serveur3-SchraVall

debug: CFLAGS += -Wall -g
debug: ${COMMON} ${HEADER}
	${CC} -o server $^ ${LFLAGS}

release: CFLAGS += -DNDEBUG
release: server.o ${COMMON} ${HEADER}
	${CC} -o server $^ ${LFLAGS}

clean:
	rm -rf server *.o
