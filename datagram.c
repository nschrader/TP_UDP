#include "datagram.h"
#include "io.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0

static Datagram _receiveDatagram(gint desc, gboolean noSequence) {
  Datagram dgram = {0};
  Address source;
  struct sockaddr* src = (struct sockaddr*) &source;
  socklen_t len = sizeof(Address);
  void* buf = noSequence ? (void*) &dgram.segment.data : (void*) &dgram.segment;

  dgram.size = recvfrom(desc, buf, sizeof(DatagramSegment), NO_FLAGS, src, &len);
  if (dgram.size == ERROR) {
    perror("Could not receive datagram");
    exit(EXIT_FAILURE);
  }

  // We need to answer, disconnect when done
  connectSocket(desc, &source);
  return dgram;
}

Datagram receiveDatagram(gint desc) {
  return _receiveDatagram(desc, FALSE);
}

Datagram receivePureData(gint desc) {
  return _receiveDatagram(desc, TRUE);
}

void sendDatagram(gint desc, const Datagram* dgram) {
  if (send(desc, &dgram->segment, dgram->size, NO_FLAGS) == ERROR) {
    perror("Could not send datagram");
    exit(EXIT_FAILURE);
  }
}

gchar* stringifyDatagramData(Datagram* dgram) {
  dgram->segment.data[SEGSIZE-1] = '\0';
  return (gchar*) dgram->segment.data;
}

void setDatagramSequence(Datagram* dgram, gint sequence) {
  g_snprintf(dgram->segment.sequence, SEQSIZE, SEQFORMAT, sequence);
}
