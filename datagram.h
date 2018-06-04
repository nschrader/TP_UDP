#ifndef DATAGRAM_H
#define DATAGRAM_H

#include "libs.h"

#define GCC_PACKED __attribute__((packed))
#define SEGSIZE 1500
#define SEQSIZE 6
#define SEQFORMAT "%06u"

typedef struct GCC_PACKED {
  gchar sequence[SEQSIZE];
  guint8 data[SEGSIZE-SEQSIZE];
} DatagramSegment;

typedef struct {
  gsize size;
  DatagramSegment segment;
} Datagram;

typedef struct sockaddr_in Address;

Datagram receiveData(gint desc);
guint receiveACK(guint *lastAck, gint desc, gint timeout, guint *dupAck);
void sendDatagram(gint desc, const Datagram* dgram);
gchar* stringifyDatagramData(Datagram* dgram);
void setDatagramSequence(Datagram* dgram, guint sequence);

#endif
