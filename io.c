#include "io.h"
#include "datagram.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR -1

int createSocket() {
  const int valid = 1;
  int desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (desc < 0) {
    perror("Could not create socket");
    exit(EXIT_FAILURE);
  }
  setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
  return desc;
}

void connectSocket(int desc, Address addr) {
  if (connect(desc, (struct sockaddr*) &addr, sizeof(addr)) == ERROR) {
    perror("Could not connect");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

void disconnectSocket(int desc) {
  Address addr;
  addr.sin_family = AF_UNSPEC;
  connectSocket(desc, addr);
}

void bindSocket(int desc, Address addr) {
  if (bind(desc, (struct sockaddr*) &addr, sizeof(addr)) == ERROR) {
    perror("Could not bind");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

Datagram readData() {
  Datagram dgram = {{0}};
  dgram.header.dataSize = fread(dgram.data, sizeof(uint8_t), SEGSIZE, stdin);
  return dgram;
}

void writeData(Datagram dgram) {
  fwrite(dgram.data, sizeof(uint8_t), dgram.header.dataSize, stdout);
}
