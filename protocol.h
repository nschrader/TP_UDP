#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "libs.h"

#define BASEPORT 1111
#define PORTRANGE 8000
#define FIRSTSEQ 000001
#define ALPHA 0.875
#define BETA 2
#define DUP_ACK_THRESH 3
#define WIN_SIZE 1
#define SSTHRESH 10

void tmntConnection(gint desc);
gint acptConnection(gint publicDesc);
void sendConnection(FILE* inputFile, gint desc);

#endif
