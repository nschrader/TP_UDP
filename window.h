#ifndef WINDOW_H
#define WINDOW_H

#include "libs.h"

void transmit(gint desc, FILE* inputFile, guint sequence, GHashTable* win);
void estimateRTT(guint* RTT, GHashTable* win, guint lastAck, guint newAckNum);
void setWin(guint ssthresh, guint* winSize, guint* t0, guint ackNum, guint RTT);
void timeoutWin(guint lastAck, GHashTable* win, guint RTO, gint desc, FILE* inputFile, guint* ssthresh, guint* winSize);

#endif
