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

guint receiveACK(guint *lastAck, gint desc, gint timeout) {
  Datagram dgram = {0};
  setSocketTimeout(desc, timeout);
  gint flags = timeout == 0 ? MSG_DONTWAIT : NO_FLAGS;
  guint newAckNum = 0;

  while (TRUE) {
    dgram.size = recv(desc, &dgram.segment.data, sizeof(DatagramSegment), flags);
    flags = MSG_DONTWAIT;
    if (dgram.size != ERROR) {
      guint ack;
      if (sscanf((gchar*) &dgram.segment.data, "ACK%06u", &ack) == ONE_MATCH) {
				if (ack <= *lastAck) {
          //TODO: Implement Fast Retransmit
					alert("fast retransmit %u", ack);
				} else {
					newAckNum = ack - *lastAck;
          *lastAck = ack;
				}
      } else {
        alert("Got some weird ACKs out here...");
      }
    } else {
      if (errno == EAGAIN) {
        if (newAckNum > 0) {
          alert("Received %u ACKs up to no %u", newAckNum, *lastAck);
        }
        return newAckNum;
      } else {
        perror("Could not receive ACK");
        close(desc);
        exit(EXIT_FAILURE);
      }
    }
  }

  return 0;
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

void setDatagramSequence(Datagram* dgram, guint sequence) {
  g_snprintf(dgram->segment.sequence, SEQSIZE, SEQFORMAT, sequence);
}
