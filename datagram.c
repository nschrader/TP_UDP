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

GList* receiveACK(GList* acks, gint desc, gint timeout) {
  Datagram dgram = {0};
  setSocketTimeout(desc, timeout);
  gint flags = timeout == 0 ? MSG_DONTWAIT : NO_FLAGS;

  while (TRUE) {
    dgram.size = recv(desc, &dgram.segment.data, sizeof(DatagramSegment), flags);
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
    close(desc);
    exit(EXIT_FAILURE);
    return NULL;
  }
}

void sendDatagram(gint desc, const Datagram* dgram) {
  if (send(desc, &dgram->segment, dgram->size, NO_FLAGS) == ERROR) {
    perror("Could not send datagram");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

gint estimateRTT (gint estimatedRTT, GList* acks, GHashTable* seqs){
	gint seqNum = GPOINTER_TO_INT(g_list_last(acks)->data);
	guint seqTime = GPOINTER_TO_UINT(g_hash_table_lookup (seqs, GINT_TO_POINTER(seqNum)));
	gint sampleTime = g_get_monotonic_time() - seqTime;
	if (estimatedRTT == 0){
		return sampleTime;
	}
	gint RTT = 0.875 * estimatedRTT + 0.125 * sampleTime;
	return RTT;
}
	

gchar* stringifyDatagramData(Datagram* dgram) {
  dgram->segment.data[sizeof(dgram->segment.data)] = '\0';
  return (gchar*) dgram->segment.data;
}

void setDatagramSequence(Datagram* dgram, gint sequence) {
  g_snprintf(dgram->segment.sequence, SEQSIZE, SEQFORMAT, sequence);
}
