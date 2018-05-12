#include "io.h"
#include "datagram.h"
#include "libs.h"

#define ERROR -1

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

void bindSocket(gint desc, const Address* addr) {
  if (bind(desc, (struct sockaddr*) addr, sizeof(*addr)) == ERROR) {
    perror("Could not bind");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

void getNameFromSocket(gint desc, const Address* addr) {
  socklen_t size = sizeof(*addr);
  getsockname(desc, (struct sockaddr*) addr, &size);
}

void setSocketTimeout(gint desc, gint milliseconds) {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000*milliseconds;
  if (setsockopt(desc, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) == ERROR) {
    perror("Could not set timeout");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

guint getMaxSeq(FILE* inputFile) {
	Datagram dgram = {0};
	fseek (inputFile, 0, SEEK_END);
	guint size = ftell(inputFile);
	return (size / sizeof(dgram.segment.data)) + 1;
}

Datagram readInputData(FILE *inputFile, guint seqNumber) {
  //TODO: This might slow down IO, we could try using a bigger buffer or so
  Datagram dgram = {0};
  fseek(inputFile, (seqNumber - 1) * sizeof(dgram.segment.data), SEEK_SET);
  dgram.size = fread(&dgram.segment.data, sizeof(guint8), sizeof(dgram.segment.data), inputFile);
  dgram.size += SEQSIZE;
  return dgram;
}
