#ifndef IO_H
#define IO_H

#include "datagram.h"
#include "libs.h"

#define ROOT_PORTS  1024

#define alert(fmt, ...) g_printf("[%d] " fmt "\n", getpid(), ##__VA_ARGS__)

gint createSocket();
void connectSocket(gint desc, const Address* addr);
void bindSocket(gint desc, const Address* addr);
void getNameFromSocket(gint desc, const Address* addr);
void setSocketTimeout(gint desc, gint milliseconds);

Datagram readInputData(FILE *inputFile, gsize seqNumber);

#endif
