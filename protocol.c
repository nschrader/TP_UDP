#include "protocol.h"
#include "io.h"
#include "datagram.h"
#include "con_status.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR -1
#define NO_FLAGS 0

static void fatalTransmissionError(const char* msg) {
  if (!errno) {
    errno = ECONNREFUSED;
  }
  perror(msg);
  exit(EXIT_FAILURE);
}

void initConnection(int desc, const char* filename) {
  Datagram syn = {{0}};
  syn.header.flags = SYN;
  strncpy((char*) syn.data, filename, SEGSIZE);
  syn.header.dataSize = strlen((char*) syn.data);
  stringifyDatagramData(&syn);
  if (send(desc, &syn, DGRAMSIZE(syn), NO_FLAGS) == ERROR) {
    goto refused;
  }

  Datagram synAck;
  if (recv(desc, &synAck, sizeof(Datagram), NO_FLAGS) == ERROR) {
    goto refused;
  }
  if (synAck.header.flags & RST) {
    goto refused;
  }
  if (!(synAck.header.flags & ACK) || !(synAck.header.flags & SYN)) {
    goto refused;
  }

  DatagramHeader ack = {0};
  ack.flags = ACK;
  if (send(desc, &ack, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto refused;
  }

  return;
  refused:
  fatalTransmissionError("Could not handshake");
}

void tmntConnection(int desc) {
  DatagramHeader fin = {0};
  fin.flags = FIN;
  if (send(desc, &fin, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto error;
  }

  Datagram finAck;
  if (recv(desc, &finAck, sizeof(Datagram), NO_FLAGS) == ERROR) {
    goto error;
  }
  if (finAck.header.flags & RST) {
    goto error;
  }
  if (!(finAck.header.flags & ACK) || !(finAck.header.flags & FIN)) {
    goto error;
  }

  DatagramHeader ack = {0};
  ack.flags = ACK;
  if (send(desc, &ack, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto error;
  }

  return;
  error:
  fatalTransmissionError("Could not handshake");
}

void acptConnection(int desc, ConStatus* status) {
  DatagramHeader synAck = {0};
  synAck.flags = SYN | ACK;
  if (send(desc, &synAck, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto error;
  }
  status->sequence = 0;

  Datagram ack;
  if (recv(desc, &ack, sizeof(Datagram), NO_FLAGS) == ERROR) {
    goto error;
  }
  if (ack.header.flags != ACK) {
    goto error;
  }
  status->acknowledgment = 0;
  status->connected = true;

  return;
  error:
  fatalTransmissionError("Could not handshake");
}

void rfseConnection(int desc) {
  DatagramHeader rst = {0};
  rst.flags = RST;
  if (send(desc, &rst, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    fatalTransmissionError("Could not reset connection");
  }
}

void clseConnection(int desc) {
  DatagramHeader finAck = {0};
  finAck.flags = FIN | ACK;
  if (send(desc, &finAck, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    goto error;
  }

  Datagram ack;
  if (recv(desc, &ack, sizeof(Datagram), NO_FLAGS) == ERROR) {
    goto error;
  }
  if (ack.header.flags != ACK) {
    goto error;
  }

  return;
  error:
  if (!errno) {
    errno = ECONNREFUSED;
  }
  fatalTransmissionError("Could not handshake");
}

void acknConnection(int desc, ConStatus* status) {
  DatagramHeader ack = {0};
  ack.flags = ACK;
  ack.sequence = status->sequence;
  if (send(desc, &ack, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    fatalTransmissionError("Could not acknowledge");
  }
}

bool lstnConnection(int desc, ConStatus *status, ProcessDatagram onAccept, ProcessDatagram onReceive, ProcessDatagram onClose) {
  disconnectSocket(desc); //Receive from everyone
  Datagram dgram = receiveDatagram(desc); //Respond only to sender

  bool closed = false;
  if (status->connected) {
    if (dgram.header.flags & SYN) {
      rfseConnection(desc);
      status->connected = false;
    } else if (dgram.header.flags & FIN) {
      clseConnection(desc);
      onClose(&dgram);
      status->connected = false;
      closed = true;
    } else if (dgram.header.flags & RST) {
      status->connected = false;
    } else if (dgram.header.sequence == status->sequence) {
      do { //First iteration won't modify dgram
        onReceive(&dgram);
        status->sequence += dgram.header.dataSize;
        //acknConnection(desc, status);
        dgram = pullSegment(status);
      } while (dgram.header.sequence); //Will be zero if segment not found
    } else {
      pushSegment(status, &dgram);
    }
  } else {
    if (dgram.header.flags & SYN) {
      acptConnection(desc, status);
      onAccept(&dgram);
    } else {
      rfseConnection(desc);
    }
  }

  return !closed;
}
