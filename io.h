#ifndef IO_H
#define IO_H

#include "datagram.h"
#include "libs.h"

#define ROOT_PORTS  1024

gint createSocket();
void connectSocket(gint desc, const Address* addr);
void disconnectSocket(gint desc);
void bindSocket(gint desc, const Address* addr);

void openInputFile(const gchar* filename);
Datagram readInputData();
void closeInputFile();
gboolean eofInputFile();

void openOutputFile(Datagram* dgram);
void closeOutputFile(Datagram* dgram);
void writeOutputData(Datagram* dgram);

#endif
