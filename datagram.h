#ifndef DATAGRAM_H
#define DATAGRAM_H

#include "libs.h"

#define MTU 1500
#define IPSIZE 32
#define UDPSIZE 8
#define GCC_PACKED __attribute__((packed))
#define MAXSEQUENCE UINT32_MAX

typedef enum GCC_PACKED {
  NONE = 0, SYN = 2, RST = 4, ACK = 8, FIN = 16
} EDatagramFlags;

typedef struct GCC_PACKED {
  EDatagramFlags flags;
  guint32 sequence;
  guint32 acknowledgment;
  guint16 dataSize;
} DatagramHeader;

#define HEADERSIZE (sizeof(DatagramHeader))
#define SEGSIZE (MTU-IPSIZE-UDPSIZE-HEADERSIZE)
#define DGRAMSIZE(d) (sizeof(guint8)*(d).header.dataSize + HEADERSIZE)

typedef struct GCC_PACKED {
  DatagramHeader header;
  guint8 data[SEGSIZE];
} Datagram;

typedef struct sockaddr_in Address;

Datagram receiveDatagram(gint desc);
void sendDatagram(gint desc, const Datagram* dgram);
void stringifyDatagramData(Datagram* dgram);

#endif
