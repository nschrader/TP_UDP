#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "datagram.h"
#include "con_status.h"
#include "libs.h"

typedef void (*ProcessDatagram)(Datagram* dgram);

//Client routines
void initConnection(gint desc, const gchar* filename);
void tmntConnection(gint desc);
void rfseConnection(gint desc);
void clseConnection(gint desc);

//Server routines
void acptConnection(gint desc, ConStatus* status);
void acknConnection(gint desc, ConStatus* status);
gboolean lstnConnection(gint desc, ConStatus* status, ProcessDatagram onAccept, ProcessDatagram onReceive, ProcessDatagram onClose);

#endif
