#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"
#include <stdbool.h>

typedef enum {
  NOT_CONNECTED, CONNECTED
} EConStatus;

typedef void (*ProcessDatagram)(Datagram dgram);

void initConnection(int desc, const char* filename);
void tmntConnection(int desc);
void acptConnection(int desc);
void rfseConnection(int desc);
void clseConnection(int desc);
bool acceptDatagram(int desc, EConStatus* status, ProcessDatagram onAccept, ProcessDatagram onReceive, ProcessDatagram onClose);

#endif
