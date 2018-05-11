#include "datagram.h"
#include "io.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0
#define ONE_MATCH 1

Datagram receiveData(gint desc) {
  Datagram dgram = {0};

  dgram.size = recv(desc, &dgram.segment.data, sizeof(DatagramSegment), NO_FLAGS);
  if (dgram.size == ERROR) {
    perror("Could not receive datagram");
    exit(EXIT_FAILURE);
  }

  return dgram;
}

gboolean receiveACK(GList** acks, gint desc, gint timeout) {
  Datagram dgram = {0};
  setSocketTimeout(desc, timeout);
  gint flags = timeout == 0 ? MSG_DONTWAIT : NO_FLAGS;
  gboolean new = FALSE;

  while (TRUE) {
    dgram.size = recv(desc, &dgram.segment.data, sizeof(DatagramSegment), flags);
    if (dgram.size != ERROR) {
      gint ack;
      if (sscanf((gchar*) &dgram.segment.data, "ACK%06d", &ack) == ONE_MATCH) {
				if (g_list_find(*acks, GINT_TO_POINTER(ack))){
					alert("fast retransmit %d", ack);
				} else {
					*acks = g_list_append(*acks, GINT_TO_POINTER(ack));
					new = TRUE;
					alert("Received ACK %d", ack);
				}	
      } else {
        alert("Got some weird ACKs out here...");
      }
    } else {
      if (errno == EAGAIN) {
        return new;
      } else {
        perror("Could not receive ACK");
        close(desc);
        exit(EXIT_FAILURE);
      }
    }
  }

  return FALSE;
}

void sendDatagram(gint desc, const Datagram* dgram) {
  if (send(desc, &dgram->segment, dgram->size, NO_FLAGS) == ERROR) {
    perror("Could not send datagram");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

gchar* stringifyDatagramData(Datagram* dgram) {
  dgram->segment.data[sizeof(dgram->segment.data)] = '\0';
  return (gchar*) dgram->segment.data;
}

void setDatagramSequence(Datagram* dgram, gint sequence) {
  g_snprintf(dgram->segment.sequence, SEQSIZE, SEQFORMAT, sequence);
}
