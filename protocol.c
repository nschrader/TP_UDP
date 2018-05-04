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

  Address addr;
  getNameFromSocket(desc, &addr);
  alert("Terminated connection to %s", inet_ntoa(addr.sin_addr));
}

gint acptConnection(gint publicDesc) {
  srand(time(NULL));
  gint port = BASEPORT + rand() % PORTRANGE;

  Address privateAddr;
  gint privateDesc = createSocket();
  privateAddr.sin_family = AF_INET;
  privateAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  privateAddr.sin_port = htons(port);
  bindSocket(privateDesc, &privateAddr);

  gchar synBuf[8];
  Address source;
  struct sockaddr* src = (struct sockaddr*) &source;
  socklen_t len = sizeof(Address);
  if (recvfrom(publicDesc, synBuf, sizeof(synBuf), NO_FLAGS, src, &len) == ERROR) {
    goto error;
  }
  alert("Got connection from %s", inet_ntoa(source.sin_addr));
  if (strncmp("SYN", synBuf, 3) != EQUALS) {
    goto error;
  }

  gchar synAckBuf[16] = "SYN-ACK0000";
  g_snprintf(&synAckBuf[7], 5, "%04d", port);
  if (sendto(publicDesc, synAckBuf, sizeof(synAckBuf), NO_FLAGS, src, len) == ERROR) {
    goto error;
  }

  gchar ackBuf[8];
  if (recvfrom(publicDesc, ackBuf, sizeof(ackBuf), NO_FLAGS, src, &len) == ERROR) {
    goto error;
  }
  if (strncmp("ACK", ackBuf, 3) != EQUALS) {
    goto error;
  }

  alert("Assigned data port %d to %s", port, inet_ntoa(source.sin_addr));
  return privateDesc;

  error:
  fatalTransmissionError("Could not handshake");
  return 0;
}

void sendConnection(FILE* inputFile, gint desc) {
  gint sequence = FIRSTSEQ;
  while (!feof(inputFile)) {
    Datagram dgram = readInputData(inputFile);
    setDatagramSequence(&dgram, sequence);
    sendDatagram(desc, &dgram);
    alert("Send seq %s", dgram.segment.sequence);
    sequence++;
  }
}
