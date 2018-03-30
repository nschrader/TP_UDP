#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"
#include "con_status.h"

typedef void (*ProcessDatagram)(Datagram* dgram);

//Client routines
void initConnection(int desc, const char* filename);
void tmntConnection(int desc);
void rfseConnection(int desc);
void clseConnection(int desc);

//Server routines
void acptConnection(int desc, ConStatus* status);
void acknConnection(int desc, ConStatus* status);
bool lstnConnection(int desc, ConStatus* status, ProcessDatagram onAccept, ProcessDatagram onReceive, ProcessDatagram onClose);

#endif
