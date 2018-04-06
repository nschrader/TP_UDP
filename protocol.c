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
  if (recv(desc, synBuf, sizeof(synBuf), NO_FLAGS) == ERROR) {
    goto error;
  }
  if (strncmp("SYN", synBuf, 3) != EQUALS) {
    goto error;
  }

  gchar synAckBuf[16] = "SYN-ACK0000";
  g_snprintf(&synAckBuf[7], 5, "%04d", port);
  if (send(desc, synAckBuf, sizeof(synAckBuf), NO_FLAGS) == ERROR) {
    goto error;
  }

  gchar ackBuf[8];
  if (recv(desc, ackBuf, sizeof(ackBuf), NO_FLAGS) == ERROR) {
    goto error;
  }
  if (strncmp("ACK", ackBuf, 3) != EQUALS) {
    goto error;
  }

  Address addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  disconnectSocket(desc);
  return addr;

  error:
  fatalTransmissionError("Could not handshake");
  return addr;
}

/*TODO: void acknConnection(gint desc, ConStatus* status) {
  DatagramHeader ack = {0};
  ack.flags = ACK;
  ack.sequence = status->sequence;
  if (send(desc, &ack, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    fatalTransmissionError("Could not acknowledge");
  }
}*/

void sendConnection(FILE* inputFile, gint desc) {
  volatile guint32 sequence = 0;
  /*TODO: while (!eofInputFile()) {
    Datagram dgram = readInputData();
    dgram.header.flags = NONE;
    dgram.header.sequence = sequence;
    dgram.header.acknowledgment = 0;
    sendDatagram(desc, &dgram);
    sequence += dgram.header.dataSize;
    g_usleep(10);
  }*/
}
