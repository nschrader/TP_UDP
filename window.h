#ifndef WINDOW_H
#define WINDOW_H

#include "libs.h"

typedef struct {
  GHashTable* win;
  guint winSize;
  guint ssthresh;
  guint RTT;
  guint RTO;
  guint t0;
} Window;

void transmit(Window* window, guint sequence, gint desc, FILE* inputFile);
void estimateRTT(Window* window, guint lastAck, guint newAckNum);
void setWin(Window* window, guint ackNum);
void timeoutWin(Window* window, guint lastAck, guint sequence, gint desc, FILE* inputFile);

#endif
