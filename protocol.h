#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"

typedef enum {
  NOT_CONNECTED, CONNECTED
} EConStatus;

typedef void (*ProcessDatagram)(Datagram dgram);

void initConnection(int desc);
void tmntConnection(int desc);
void acptConnection(int desc);
void rfseConnection(int desc);
void clseConnection(int desc);
EConStatus acceptDatagram(int desc, EConStatus status, ProcessDatagram success);

#endif
