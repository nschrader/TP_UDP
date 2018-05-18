#include "window.h"
#include "datagram.h"
#include "io.h"
#include "protocol.h"
#include "libs.h"

void transmit(Window* window, guint sequence, gint desc, FILE* inputFile) {
  Datagram dgram = readInputData(inputFile, sequence);
  setDatagramSequence(&dgram, sequence);
  sendDatagram(desc, &dgram);
  g_hash_table_replace(window->win, GUINT_TO_POINTER(sequence), GUINT_TO_POINTER(getMonotonicTimeSave()));
}

void estimateRTT(Window* window, guint lastAck, guint newAckNum) {
  for (guint seqNum = (lastAck-newAckNum+1); seqNum <= lastAck; seqNum++) {
  	guint seqTime = GPOINTER_TO_UINT(g_hash_table_lookup(window->win, GINT_TO_POINTER(seqNum)));
    assert(seqTime != 0);
    guint sampleTime = getMonotonicTimeSave() - seqTime;
    if (window->RTT == 0) {
  		window->RTT = sampleTime;
  	} else {
      window->RTT = ALPHA * window->RTT + (1-ALPHA) * sampleTime;
    }
    window->RTO = BETA * window->RTT;
  }
}

void setWin(Window* window, guint ackNum) {
	if (window->winSize == window->ssthresh) {
		window->t0 = getMonotonicTimeSave();
		window->winSize += ackNum;
	} else if (window->winSize < window->ssthresh) {
		window->winSize += ackNum;
	} else if (window->winSize > window->ssthresh && (getMonotonicTimeSave() - window->t0) > window->RTT) {
    window->t0 = getMonotonicTimeSave();
		window->winSize += 1;
	}
}

static gboolean iterSeqRem(gpointer key, gpointer value, gpointer last) {
  return GPOINTER_TO_UINT(key) <= GPOINTER_TO_UINT(last);
}

void timeoutWin(Window* window, guint lastAck, guint sequence, gint desc, FILE* inputFile) {
	g_hash_table_foreach_remove(window->win, iterSeqRem, GUINT_TO_POINTER(lastAck));

  //TODO: Maybe we should iterate from lastAck upto sequence and respect winsize
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, window->win);
	gboolean timeout = FALSE;
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		guint time = GPOINTER_TO_UINT(value);
		if (getMonotonicTimeSave() - time >= window->RTO) {
			alert("seq %u timeout - sending again ...", GPOINTER_TO_UINT(key));
			transmit(window, GPOINTER_TO_UINT(key), desc, inputFile);
			timeout = TRUE;
		}
	}
	if (timeout) {
    //TODO: Maybe it's more efficient to avoid resetting ssthresh to 1 because window was reset to 1 just before
		window->ssthresh = (window->winSize/2) + 1;
		window->winSize = 1;
	}
}
