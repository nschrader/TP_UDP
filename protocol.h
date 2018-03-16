#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"

typedef enum {
  NOT_CONNECTED, CONNECTED
} EConStatus;

typedef void (*ProcessDatagram)(Datagram dgram);

void Init3WayHandshake(int desc);
void Acpt3WayHandshake(int desc);
void Rfse3WayHandshake(int desc);
EConStatus acceptDatagram(int desc, EConStatus status, ProcessDatagram success);

#endif
