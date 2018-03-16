#ifndef IO_H
#define IO_H

#include "datagram.h"

#define ROOT_PORTS  1024

int createSocket();
void connectSocket(int desc, Address addr);
void disconnectSocket(int desc);
void bindSocket(int desc, Address addr);

Datagram readData();
void writeData(Datagram dgram);

#endif
