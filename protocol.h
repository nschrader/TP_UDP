#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"
#include <stdbool.h>

typedef struct {
  bool connected;
  uint32_t sequence;
  uint32_t acknowledgment;
  Datagram* stack[MAX_SEQUENCE / SEGSIZE];
} ConStatus;

typedef void (*ProcessDatagram)(Datagram dgram);

void initConnection(int desc, const char* filename);
void tmntConnection(int desc);
void acptConnection(int desc, ConStatus* status);
void rfseConnection(int desc);
void clseConnection(int desc);
void acknConnection(int desc, uint32_t sequence);
bool acceptDatagram(int desc, ConStatus* status, ProcessDatagram onAccept, ProcessDatagram onReceive, ProcessDatagram onClose);

#endif
