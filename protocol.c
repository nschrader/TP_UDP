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
  guint lastAck = 0;
  guint sequence = FIRSTSEQ;
	guint maxSeq = getMaxSeq(inputFile);

  Window window = {
    .win = g_hash_table_new(g_direct_hash, g_direct_equal),
    .winSize = WIN_SIZE,
    .ssthresh = SSTHRESH,
    .RTT = 0,
    .RTO = USECS_IN_SEC,
    .t0 = 0
  };

  while (lastAck != maxSeq) {
  	guint timeout = (sequence <= maxSeq && g_hash_table_size(window.win) < window.winSize) ? 0 : window.RTO;
    guint dupAck = 0;
    guint newAckNum = receiveACK(&lastAck, desc, timeout, &dupAck);
    if(newAckNum  > 0) {
      estimateRTT(&window, lastAck, newAckNum);
      alert("RTT is now: %.0fms", window.RTT/1000.0);
      setWin(&window, newAckNum);
    }

    //TODO: Maybe we should not do either or, but both
    if (dupAck > DUP_ACK_THRESH) {
      alert("%u ACK duplicates of %u detected - Fast Retransmit of %u...", dupAck, lastAck, lastAck+1);
      transmit(&window, lastAck+1, desc, inputFile);
		} else {
      timeoutWin(&window, lastAck, sequence, desc, inputFile);
  		while (sequence <= maxSeq && g_hash_table_size(window.win) < window.winSize) {
        alert("Send seq %u", sequence);
  			transmit(&window, sequence, desc, inputFile);
  			sequence++;
  		}
    }
  }

  g_hash_table_destroy(window.win);
}
