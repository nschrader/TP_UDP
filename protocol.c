#include "protocol.h"
#include "io.h"
#include "datagram.h"
#include "window.h"
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

void sendConnection(FILE* inputFile, gint desc) {
  GHashTable* win = g_hash_table_new(g_direct_hash, g_direct_equal);
  guint lastAck = 0;
  guint sequence = FIRSTSEQ;
	guint RTT = 0;
	guint RTO = USECS_IN_SEC;
	guint maxSeq = getMaxSeq(inputFile);

	guint winSize = WIN_SIZE;
	guint ssthresh = THRESH;
	guint t0 = 0;

  while (lastAck != maxSeq) {
  	guint timeout = (sequence <= maxSeq && g_hash_table_size(win) < winSize) ? 0 : RTO;
    guint dupAck = 0;
    guint newAckNum = receiveACK(&lastAck, desc, timeout, &dupAck);
    if(newAckNum  > 0) {
      estimateRTT(&RTT, win, lastAck, newAckNum);
			RTO = BETA * RTT;
      alert("RTT is now: %.0fms", RTT/1000.0);
      setWin(ssthresh, &winSize, &t0, newAckNum, RTT);
    }

    if (dupAck > DUP_ACK_THRESH) {
      alert("%u ACK duplicates of %u detected - Fast Retransmit of %u...", dupAck, lastAck, lastAck+1);
      transmit(desc, inputFile, lastAck+1, win);
    }
    
    timeoutWin(lastAck, win, RTO, desc, inputFile, &ssthresh, &winSize);
		while (sequence <= maxSeq && g_hash_table_size(win) < winSize) {
      alert("Send seq %u", sequence);
			transmit(desc, inputFile, sequence, win);
			sequence++;
		}
  }

  g_hash_table_destroy(win);
}
