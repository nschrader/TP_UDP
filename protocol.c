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

  Address privateAddr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = htonl(INADDR_ANY),
    .sin_port = htons(port)
  };
  gint privateDesc = createSocket();
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

  connectSocket(privateDesc, &source);

  alert("Assigned data port %d to %s", port, inet_ntoa(source.sin_addr));
  return privateDesc;

  error:
  fatalTransmissionError("Could not handshake");
  return 0;
}

//TODO: Remove those iterators
void iterAcks(gpointer data, gpointer user_data) {
  alert("no %d", GPOINTER_TO_INT(data));
}

void iterSeqs(gpointer key, gpointer value, gpointer user_data) {
  alert("no %d at %u", GPOINTER_TO_INT(key), GPOINTER_TO_UINT(value));
}

void sendConnection(FILE* inputFile, gint desc) {
  gint sequence = FIRSTSEQ;
  GList* acks = NULL;
  GHashTable* seqs = g_hash_table_new(g_direct_hash, g_direct_equal);

  while (!feof(inputFile)) {
    acks = receiveACK(acks, desc);
    usleep(100000); //Otherwise we quit so fast that we won't even receive

    Datagram dgram = readInputData(inputFile);
    setDatagramSequence(&dgram, sequence);
    sendDatagram(desc, &dgram);
    g_hash_table_insert(seqs, GINT_TO_POINTER(sequence), GUINT_TO_POINTER(g_get_monotonic_time()));
    alert("Send seq %s", dgram.segment.sequence);
    sequence++;
  }

  alert("Got the following ACKs:");
  g_list_foreach(acks, iterAcks, NULL);
  alert("Send the following sequences:");
  g_hash_table_foreach(seqs, iterSeqs, NULL);

  g_list_free(acks);
  g_hash_table_destroy(seqs);
}
