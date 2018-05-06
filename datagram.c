#include "datagram.h"
#include "io.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0
#define ONE_MATCH 1

Datagram receiveData(gint desc) {
  Datagram dgram = {0};
  Address source;
  struct sockaddr* src = (struct sockaddr*) &source;
  socklen_t len = sizeof(Address);

  dgram.size = recvfrom(desc, &dgram.segment.data, sizeof(DatagramSegment), NO_FLAGS, src, &len);
  if (dgram.size == ERROR) {
    perror("Could not receive datagram");
    exit(EXIT_FAILURE);
  }

  // We need to answer, disconnect when done
  connectSocket(desc, &source);
  return dgram;
}

GList* receiveACK(guint desc) {
  Datagram dgram = {0};
  GList* acks = NULL;

  while (TRUE) {
    dgram.size = recv(desc, &dgram.segment.data, sizeof(DatagramSegment), MSG_DONTWAIT);
    if (dgram.size == ERROR) {
      break;
    }

    gint ack;
    if (sscanf((gchar*) &dgram.segment.data, "ACK%06d", &ack) == ONE_MATCH) {
      acks = g_list_append(acks, GINT_TO_POINTER(ack));
    } else {
      alert("Got some weird ACKs out here...");
    }
  }

  if (errno == EAGAIN) {
    return acks;
  } else {
    perror("Could not receive ACK");
    exit(EXIT_FAILURE);
    return NULL;
  }
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
