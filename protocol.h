#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"
#include "libs.h"

#define BASEPORT 1111
#define PORTRANGE 8000
#define FIRSTSEQ 000001

void tmntConnection(gint desc);
Address acptConnection(gint desc);
void sendConnection(FILE* inputFile, gint desc);

#endif
