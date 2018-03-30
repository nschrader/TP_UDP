#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include "datagram.h"

#define ROOT_PORTS  1024

int createSocket();
void connectSocket(int desc, const Address* addr);
void disconnectSocket(int desc);
void bindSocket(int desc, const Address* addr);

void openInputFile(const char* filename);
Datagram readInputData();
void closeInputFile();
bool eofInputFile();

void openOutputFile(Datagram* dgram);
void closeOutputFile(Datagram* dgram);
void writeOutputData(Datagram* dgram);

#endif
