#ifndef MISC_H
#define MISC_H

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SEGSIZE 1024
#define GCC_PACKED __attribute__((packed))
#define DGRAMSIZE(d) (sizeof(char)*(strlen(d.data)+2)+sizeof(int))

typedef enum GCC_PACKED {
  NONE = 0, SYN = 2, RST = 4, ACK = 8
} EDatagramFlags;

typedef struct GCC_PACKED {
  EDatagramFlags flags;
  int segment;
  char data[SEGSIZE];
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
void sendDatagram(int desc);
void Init3WayHandshake(int desc);
void Acpt3WayHandshake(int desc);
void Rfse3WayHandshake(int desc);

#endif
