#ifndef CON_STATUS_H
#define CON_STATUS_H

#include "datagram.h"
#include "libs.h"

typedef struct {
  gboolean connected;
  guint32 sequence;
  guint32 acknowledgment;
  GQueue* stack;
} ConStatus;

ConStatus* newConStatus();
void freeConStatus(ConStatus* status);
void pushSegment(ConStatus* status, Datagram* dgram);
Datagram pullSegment(ConStatus* status);

#endif
