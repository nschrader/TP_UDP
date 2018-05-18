#include "window.h"
#include "datagram.h"
#include "io.h"
#include "protocol.h"
#include "libs.h"

void transmit(gint desc, FILE* inputFile, guint sequence, GHashTable* win) {
  Datagram dgram = readInputData(inputFile, sequence);
  setDatagramSequence(&dgram, sequence);
  sendDatagram(desc, &dgram);
  g_hash_table_replace(win, GUINT_TO_POINTER(sequence), GUINT_TO_POINTER(getMonotonicTimeSave()));
}

void estimateRTT(guint* RTT, GHashTable* win, guint lastAck, guint newAckNum) {
  for (guint seqNum = (lastAck-newAckNum+1); seqNum <= lastAck; seqNum++) {
  	guint seqTime = GPOINTER_TO_UINT(g_hash_table_lookup(win, GINT_TO_POINTER(seqNum)));
    assert(seqTime != 0);
    guint sampleTime = getMonotonicTimeSave() - seqTime;
    if (*RTT == 0) {
  		*RTT = sampleTime;
  	} else {
      *RTT = ALPHA * (*RTT) + (1-ALPHA) * sampleTime;
    }
  }
}

void setWin(guint ssthresh, guint* winSize, guint* t0, guint ackNum, guint RTT) {
	if (*winSize == ssthresh) {
		*t0 = getMonotonicTimeSave();
		*winSize += ackNum;
	} else if (*winSize < ssthresh) {
		*winSize += ackNum;
	} else if (*winSize > ssthresh && (getMonotonicTimeSave() - *t0) > RTT) {
    *t0 = getMonotonicTimeSave();
		*winSize += 1;
	}
}

static gboolean iterSeqRem(gpointer key, gpointer value, gpointer last) {
  return GPOINTER_TO_UINT(key) <= GPOINTER_TO_UINT(last);
}

void timeoutWin(guint lastAck, GHashTable* win, guint RTO, gint desc, FILE* inputFile, guint* ssthresh, guint* winSize) {
	g_hash_table_foreach_remove(win, iterSeqRem, GUINT_TO_POINTER(lastAck));

  //TODO: Maybe we should iterate from lastAck upto sequence and respect winsize
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, win);
	gboolean timeout = FALSE;
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		guint time = GPOINTER_TO_UINT(value);
		if (getMonotonicTimeSave() - time >= RTO) {
			alert("seq %u timeout - sending again ...", GPOINTER_TO_UINT(key));
			transmit(desc, inputFile, GPOINTER_TO_UINT(key), win);
			timeout = TRUE;
		}
	}
	if (timeout) {
    //TODO: Maybe it's more efficient to avoid resetting ssthresh to 1 because window was reset to 1 just before
		*ssthresh = (*winSize)/2 + 1;
		*winSize = 1;
	}
}
