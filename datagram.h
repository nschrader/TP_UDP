#ifndef DATAGRAM_H
#define DATAGRAM_H

#include "libs.h"


#define GCC_PACKED __attribute__((packed))
#define BYTESIZE 8
#define HEADERSIZE 6
#define SEGSIZE 1500

typedef struct GCC_PACKED {
  guint64 sequence: HEADERSIZE*BYTESIZE;
  guint8 data[SEGSIZE-HEADERSIZE];
} DatagramSegment;

typedef struct {
  gsize size;
  DatagramSegment segment;
} Datagram;

typedef struct sockaddr_in Address;

Datagram receiveDatagram(gint desc);
void sendDatagram(gint desc, const Datagram* dgram);
gchar* stringifyDatagramData(Datagram* dgram);

#endif
