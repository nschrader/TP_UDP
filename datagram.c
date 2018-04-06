#include "datagram.h"
#include "io.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0

Datagram receiveDatagram(gint desc) {
  Datagram dgram = {0};
  Address source;
  struct sockaddr* src = (struct sockaddr*) &source;
  socklen_t len = sizeof(Address);

  dgram.size = recvfrom(desc, &dgram.segment, sizeof(DatagramSegment), NO_FLAGS, src, &len);
  if (dgram.size == ERROR) {
    perror("Could not receive datagram");
    exit(EXIT_FAILURE);
  }

  // We need to answer, disconnect when done
  connectSocket(desc, &source);
  return dgram;
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
