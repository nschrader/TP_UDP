#ifndef MISC_H
#define MISC_H

#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MTU 1500
#define IPSIZE 32
#define UDPSIZE 8
#define GCC_PACKED __attribute__((packed))

typedef enum GCC_PACKED {
  NONE = 0, SYN = 2, RST = 4, ACK = 8
} EDatagramFlags;

typedef struct GCC_PACKED {
  EDatagramFlags flags;
  uint32_t segment;
  uint32_t acknowledgment;
  uint16_t dataSize;
} DatagramHeader;

#define HEADERSIZE (sizeof(DatagramHeader))
#define SEGSIZE (MTU-IPSIZE-UDPSIZE-HEADERSIZE)
#define DGRAMSIZE(d) (sizeof(uint8_t)*d.header.dataSize + HEADERSIZE)

typedef struct GCC_PACKED {
  DatagramHeader header;
  uint8_t data[SEGSIZE];
} Datagram;

typedef enum {
  NOT_CONNECTED, CONNECTED
} EConStatus;

typedef struct sockaddr_in Address;

int createSocket();
void connectSocket(int desc, Address addr);
void disconnectSocket(int desc);
void bindSocket(int desc, Address addr);
void ResetSocket(int desc);
Datagram receiveDatagram(int desc);
void sendDatagram(int desc, Datagram dgram);
void Init3WayHandshake(int desc);
void Acpt3WayHandshake(int desc);
void Rfse3WayHandshake(int desc);
Datagram readData();
void writeData(Datagram dgram);

#endif
