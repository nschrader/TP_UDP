#include "protocol.h"
#include "io.h"
#include "datagram.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0
#define EQUALS 0

#define ALPHA 0.875
#define BETA 2

#define SEC_IN_USEC (1000*1000)

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

static void estimateRTT(guint* RTT, GHashTable* seqs, GQueue* acks, guint newAckNum) {
  guint length = g_queue_get_length(acks);
  for (guint n = (length-newAckNum); n < length; n++) {
    guint seqNum = GPOINTER_TO_UINT(g_queue_peek_nth(acks, n));
  	guint seqTime = GPOINTER_TO_UINT(g_hash_table_lookup(seqs, GINT_TO_POINTER(seqNum)));
    guint sampleTime = g_get_monotonic_time() - seqTime;
    if (*RTT == 0) {
  		*RTT = sampleTime;
  	} else {
      *RTT = ALPHA * (*RTT) + (1-ALPHA) * sampleTime;
    }
  }
}

gboolean iterSeqRem(gpointer key, gpointer value, gpointer packs) {
	GQueue* acks = (GQueue*) packs;
	if (g_queue_find(acks, key) != NULL) {
		for (guint i = 1; i < GPOINTER_TO_UINT(key); i++) {
			g_queue_remove(acks, GUINT_TO_POINTER(i));
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

static void majSeq(GQueue* acks, GHashTable* seqs, guint RTO, gint desc, FILE* inputFile) {
	g_hash_table_foreach_remove(seqs, iterSeqRem, acks); //remove what is OK

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, seqs);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		guint time = GPOINTER_TO_UINT(value);
		if (g_get_monotonic_time() - time >= RTO) {
			alert("seq %u timeout - sending again ...", GPOINTER_TO_UINT(key));
			Datagram dgram = readInputData(inputFile, GPOINTER_TO_UINT(key));
			setDatagramSequence(&dgram, GPOINTER_TO_UINT(key));
			sendDatagram(desc, &dgram);
		}
	}
}

void sendConnection(FILE* inputFile, gint desc) {
  GQueue* acks = g_queue_new();
  GHashTable* seqs = g_hash_table_new(g_direct_hash, g_direct_equal);
  guint sequence = FIRSTSEQ;
	guint RTT = 0;
	guint RTO = SEC_IN_USEC;
	guint maxSeq = getMaxSeq(inputFile);

  while (GPOINTER_TO_UINT(g_queue_peek_tail(acks)) != maxSeq) {
    guint newAckNum = receiveACK(acks, desc, 100);
    if(newAckNum  > 0) {
      estimateRTT(&RTT, seqs, acks, newAckNum);
			RTO = BETA * RTT;
      alert("RTT is now: %.0fms", RTT/1000.0);
    }
    //TODO: TO be removed
    usleep(100000); //Otherwise we quit so fast that we won't even receive

		majSeq(acks, seqs, RTO, desc, inputFile);

		if (sequence <= maxSeq){
			Datagram dgram = readInputData(inputFile, sequence);
			setDatagramSequence(&dgram, sequence);
			sendDatagram(desc, &dgram);
			g_hash_table_insert(seqs, GUINT_TO_POINTER(sequence), GUINT_TO_POINTER(g_get_monotonic_time()));
			alert("Send seq %s", dgram.segment.sequence);
			sequence++;
		}
  }

  g_queue_free(acks);
  g_hash_table_destroy(seqs);
}
