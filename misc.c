#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR -1
#define NO_FLAGS 0

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

void ResetSocket(int desc) {
  DatagramHeader rst = {0};
  rst.flags = RST;
  if (send(desc, &rst, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    perror("Could reset client");
    exit(EXIT_FAILURE);
  }
}

Datagram receiveDatagram(int desc) {
  Datagram dgram = {{0}};
  Address source;
  struct sockaddr* src = (struct sockaddr*) &source;
  socklen_t len = sizeof(Address);

  if (recvfrom(desc, &dgram, sizeof(Datagram), NO_FLAGS, src, &len) == ERROR) {
    perror("Could not receive datagram");
    exit(EXIT_FAILURE);
  }

  connect(desc, src, len);
  return dgram;
}

void sendDatagram(int desc, Datagram dgram) {
  if (send(desc, &dgram, DGRAMSIZE(dgram), NO_FLAGS) == ERROR) {
    perror("Could not send datagram");
    exit(EXIT_FAILURE);
  }
}

void Init3WayHandshake(int desc) {
  DatagramHeader syn = {0};
  syn.flags = SYN;
  if (send(desc, &syn, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto refused;
  }

  Datagram synAck;
  if (recv(desc, &synAck, sizeof(Datagram), NO_FLAGS) == ERROR) {
    goto refused;
  }

  if (synAck.header.flags & RST) {
    goto refused;
  }
  if (!(synAck.header.flags & ACK) || !(synAck.header.flags & SYN)) {
    goto refused;
  }

  DatagramHeader ack = {0};
  ack.flags = ACK;
  if (send(desc, &ack, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto refused;
  }

  return;

  refused:
  if (!errno) {
    errno = ECONNREFUSED;
  }
  perror("Could not handshake");
  exit(EXIT_FAILURE);
}

void Acpt3WayHandshake(int desc) {
  DatagramHeader synAck = {0};
  synAck.flags = SYN | ACK;
  if (send(desc, &synAck, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    perror("Could not handshake");
    exit(EXIT_FAILURE);
  }
}

void Rfse3WayHandshake(int desc) {
  Datagram rst = {{0}};
  rst.header.flags = RST;
  if (send(desc, &rst, DGRAMSIZE(rst), NO_FLAGS) == ERROR) {
    perror("Could not handshake");
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
