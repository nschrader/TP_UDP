#include "datagram.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR -1
#define NO_FLAGS 0

void resetConnection(int desc) {
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
