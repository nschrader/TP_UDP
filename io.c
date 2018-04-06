#include "io.h"
#include "datagram.h"
#include "libs.h"

#define ERROR -1

FILE* inputFile;
FILE* outputFile;

gint createSocket() {
  const gint valid = 1;
  gint desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (desc < 0) {
    perror("Could not create socket");
    exit(EXIT_FAILURE);
  }
  setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(gint));
  return desc;
}

void connectSocket(gint desc, const Address* addr) {
  if (connect(desc, (struct sockaddr*) addr, sizeof(*addr)) == ERROR) {
    perror("Could not connect");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

void disconnectSocket(gint desc) {
  Address addr;
  addr.sin_family = AF_UNSPEC;
  connectSocket(desc, &addr);
}

void bindSocket(gint desc, const Address* addr) {
  if (bind(desc, (struct sockaddr*) addr, sizeof(*addr)) == ERROR) {
    perror("Could not bind");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

Datagram readInputData() {
  Datagram dgram = {0};
  dgram.size = fread(dgram.segment.data, sizeof(guint8), sizeof(dgram.segment.data), inputFile);
  return dgram;
}
