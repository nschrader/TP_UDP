#ifndef MISC_H
#define MISC_H

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SEGSIZE 1024
#define GCC_PACKED __attribute__((packed))
#define DGRAMSIZE(d) (sizeof(char)*(strlen(d.data)+2)+sizeof(int))

typedef enum {
  NONE, SYN, ACK
} DatagramFlags;

typedef struct GCC_PACKED {
  DatagramFlags flags;
  int segment;
  char data[SEGSIZE];
} Datagram;

int createSocket();
void connectSocket(int desc, struct sockaddr_in addr);
void bindSocket(int desc, struct sockaddr_in addr);
void recieveSocket(int desc);
void sendSocket(int desc);

#endif
