#ifndef IO_H
#define IO_H

#include <stdio.h>
#include "datagram.h"

#define ROOT_PORTS  1024

int createSocket();
void connectSocket(int desc, Address addr);
void disconnectSocket(int desc);
void bindSocket(int desc, Address addr);

FILE* openFile(char* filename)
Datagram readData(FILE* file);
void openDataFile(char* filename);
void closeDataFile();
void writeData(Datagram dgram);

#endif
