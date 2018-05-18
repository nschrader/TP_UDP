//TODO: Maybe put stuff into another file, like window.c
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

static void transmit(gint desc, FILE* inputFile, guint sequence, GHashTable* seqs) {
  Datagram dgram = readInputData(inputFile, sequence);
  setDatagramSequence(&dgram, sequence);
  sendDatagram(desc, &dgram);
  g_hash_table_replace(seqs, GUINT_TO_POINTER(sequence), GUINT_TO_POINTER(getMonotonicTimeSave()));
}

static void estimateRTT(guint* RTT, GHashTable* seqs, guint lastAck, guint newAckNum) {
  for (guint seqNum = (lastAck-newAckNum+1); seqNum <= lastAck; seqNum++) {
  	guint seqTime = GPOINTER_TO_UINT(g_hash_table_lookup(seqs, GINT_TO_POINTER(seqNum)));
    assert(seqTime != 0);
    guint sampleTime = getMonotonicTimeSave() - seqTime;
    if (*RTT == 0) {
  		*RTT = sampleTime;
  	} else {
      *RTT = ALPHA * (*RTT) + (1-ALPHA) * sampleTime;
    }
  }
}

static gboolean iterSeqRem(gpointer key, gpointer value, gpointer last) {
  return GPOINTER_TO_UINT(key) <= GPOINTER_TO_UINT(last);
}

//TODO: Clean signature up
static void majSeq(guint lastAck, GHashTable* seqs, guint RTO, gint desc, FILE* inputFile, guint* ssthresh, guint* winSize) {
	g_hash_table_foreach_remove(seqs, iterSeqRem, GUINT_TO_POINTER(lastAck));

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, seqs);
	gboolean timeout = FALSE;
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		guint time = GPOINTER_TO_UINT(value);
		if (getMonotonicTimeSave() - time >= RTO) {
			alert("seq %u timeout - sending again ...", GPOINTER_TO_UINT(key));
			transmit(desc, inputFile, GPOINTER_TO_UINT(key), seqs);
			timeout = TRUE;
		}
	}
	if (timeout) {
		*ssthresh = (*winSize)/2 + 1;
		*winSize = 1;
		alert("winsize reset");
	}
}

//TODO: Clean this up
//Seems buggy, evolution of contention window is not as I would have expected
void setWin(guint ssthresh, guint* winSize, guint* t0, guint ackNum, guint RTT) {
	if (*winSize == ssthresh) {
		*t0 = getMonotonicTimeSave();
		*winSize += ackNum;
	} else if (*winSize < ssthresh) {
		*winSize += ackNum;
	} else if (*winSize > ssthresh && (getMonotonicTimeSave() - *t0) > RTT) {
		*winSize += 1;
		*t0 = getMonotonicTimeSave();
	}
}

void sendConnection(FILE* inputFile, gint desc) {
  //TODO: Rename this to "win"
  GHashTable* seqs = g_hash_table_new(g_direct_hash, g_direct_equal);
  guint lastAck = 0;
  guint sequence = FIRSTSEQ;
	guint RTT = 0;
	guint RTO = USECS_IN_SEC;
	guint maxSeq = getMaxSeq(inputFile);

	guint winSize = 1;
	guint ssthresh = 10;
	guint t0 = 0;

  while (lastAck != maxSeq) {
    //TODO: At the end of transmission timeout falls down to zero because
    //contention window is big enough, but no more sequences are available.
    //This generates a lot of processor load...
  	guint timeout = g_hash_table_size(seqs) < winSize ? 0 : RTO;
    //TODO: Remove
    guint newAckNum = receiveACK(&lastAck, desc, timeout);
    if(newAckNum  > 0) {
      estimateRTT(&RTT, seqs, lastAck, newAckNum);
			RTO = BETA * RTT;
      alert("RTT is now: %.0fms", RTT/1000.0);
      setWin(ssthresh, &winSize, &t0, newAckNum, RTT);
    }

		majSeq(lastAck, seqs, RTO, desc, inputFile, &ssthresh, &winSize);
		alert("winSize %u, seqSize %u, ssthresh %u, timeout %u", winSize, g_hash_table_size(seqs), ssthresh, timeout);
    
		while (sequence <= maxSeq && g_hash_table_size(seqs) < winSize) {
      alert("Send seq %u", sequence);
			transmit(desc, inputFile, sequence, seqs);
			sequence++;
		}
  }

  g_hash_table_destroy(seqs);
}
