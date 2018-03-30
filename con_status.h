#ifndef CON_STATUS_H
#define CON_STATUS_H

#include "datagram.h"

#include <stdbool.h>
#include <stdint.h>
#include <gmodule.h>

typedef struct {
  bool connected;
  uint32_t sequence;
  uint32_t acknowledgment;
  GQueue* stack;
} ConStatus;

ConStatus* newConStatus();
void freeConStatus(ConStatus* status);
void pushSegment(ConStatus* status, Datagram* dgram);
Datagram pullSegment(ConStatus* status);

#endif
