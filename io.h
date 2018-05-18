#ifndef IO_H
#define IO_H

#include "datagram.h"
#include "libs.h"

#define ROOT_PORTS  1024
#define USECS_IN_SEC (1000*1000)

//TODO: Maybe we will have less problems with output if the stdout buffer would be bigger
#ifndef NDEBUG
#define alert(fmt, ...) g_printf("[%d] " fmt "\n", getpid(), ##__VA_ARGS__)
#else
#define alert(ignore, ...) ((void) 0)
#endif

gint createSocket();
void connectSocket(gint desc, const Address* addr);
void bindSocket(gint desc, const Address* addr);
void getNameFromSocket(gint desc, const Address* addr);
void setSocketTimeout(gint desc, gint useconds);
guint getMaxSeq(FILE* inputFile);

Datagram readInputData(FILE *inputFile, guint seqNumber);
guint getMonotonicTimeSave();
#endif
