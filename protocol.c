#include "protocol.h"
#include "io.h"
#include "datagram.h"
#include "libs.h"

#define ERROR -1
#define NO_FLAGS 0
#define EQUALS 0

#define ALPHA 0.875
#define BETA 2

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

static guint estimateRTT(gint estimatedRTT, GList* acks, GHashTable* seqs){
	gint seqNum = GPOINTER_TO_INT(g_list_last(acks)->data);
	guint seqTime = GPOINTER_TO_UINT(g_hash_table_lookup(seqs, GINT_TO_POINTER(seqNum)));
  guint sampleTime = g_get_monotonic_time() - seqTime;
	if (estimatedRTT == 0) {
		return sampleTime;
	}
	gint RTT = ALPHA * estimatedRTT + (1-ALPHA) * sampleTime;
	return RTT;
}

gboolean iterSeqRem(gpointer key, gpointer value, gpointer packs) {
	GList* acks = (GList*) packs;
	return g_list_find (acks, key) != NULL; // !!! remove the acks too
}


static void majSeq(GList* acks, GHashTable* seqs, gint RTO, gint desc, FILE* inputFile) {
	g_hash_table_foreach_remove (seqs, iterSeqRem, acks); //remove what is OK
	
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, seqs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
			guint time = GPOINTER_TO_UINT (value);
			if (g_get_monotonic_time() - time >= RTO) {
				alert("seq %d timeout - sending again ...", GPOINTER_TO_INT(key)); 
				Datagram dgram = {0};
				dgram.size = sizeof(dgram.segment);
				setDatagramSequence(&dgram, GPOINTER_TO_INT(key));
				sendDatagram(desc, &dgram);
			}
		}
}

void sendConnection(FILE* inputFile, gint desc) {
  gint sequence = FIRSTSEQ;
  GList* acks = NULL;
  GHashTable* seqs = g_hash_table_new(g_direct_hash, g_direct_equal);
	gint RTT = 0;
	gint RTO = 1000000000;

  while (!feof(inputFile)) {
    if(receiveACK(&acks, desc, 100)) {
      RTT = estimateRTT(RTT, acks, seqs);
			alert("RTT: %d", RTT);
			RTO = BETA * RTT;
    }
    usleep(100000); //Otherwise we quit so fast that we won't even receive
	
		majSeq(acks, seqs, RTO, desc, inputFile);
		
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
