#include "protocol.h"
#include "io.h"
#include "datagram.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0
#define EQUALS 0

static void fatalTransmissionError(const gchar* msg) {
  if (!errno) {
    errno = ECONNREFUSED;
  }
  perror(msg);
  exit(EXIT_FAILURE);
}

void tmntConnection(gint desc) {
  gchar finBuf[16] = "FIN";
  if (send(desc, finBuf, sizeof(finBuf), NO_FLAGS) == ERROR) {
    fatalTransmissionError("Could not handshake");
  }
}

Address acptConnection(gint desc) {
  //TODO: Get first free port from bind
  srand(time(NULL));
  gint port = BASEPORT + rand() % PORTRANGE;

  gchar synBuf[8];
  Address source;
  struct sockaddr* src = (struct sockaddr*) &source;
  socklen_t len = sizeof(Address);
  if (recvfrom(desc, synBuf, sizeof(synBuf), NO_FLAGS, src, &len) == ERROR) {
    goto error;
  }
  if (strncmp("SYN", synBuf, 3) != EQUALS) {
    goto error;
  }

  gchar synAckBuf[16] = "SYN-ACK0000";
  g_snprintf(&synAckBuf[7], 5, "%04d", port);
  if (sendto(desc, synAckBuf, sizeof(synAckBuf), NO_FLAGS, src, len) == ERROR) {
    goto error;
  }

  gchar ackBuf[8];
  if (recvfrom(desc, ackBuf, sizeof(ackBuf), NO_FLAGS, src, &len) == ERROR) {
    goto error;
  }
  if (strncmp("ACK", ackBuf, 3) != EQUALS) {
    goto error;
  }

  Address addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  //TODO: Might be waste
  //disconnectSocket(desc);
  return addr;

  error:
  fatalTransmissionError("Could not handshake");
  return addr;
}

void sendConnection(FILE* inputFile, gint desc) {
  gint sequence = FIRSTSEQ;
  while (!feof(inputFile)) {
    Datagram dgram = readInputData(inputFile);
    setDatagramSequence(&dgram, sequence);
    sendDatagram(desc, &dgram);
    //TODO: Remove
    g_printf("Send seq %s\n", dgram.segment.sequence);
    sequence++;
    //TODO: Remove
    g_usleep(10);
  }
}
