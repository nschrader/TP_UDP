#include "protocol.h"
#include "io.h"
#include "datagram.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR -1
#define NO_FLAGS 0

void Init3WayHandshake(int desc) {
  DatagramHeader syn = {0};
  syn.flags = SYN;
  if (send(desc, &syn, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
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
  if (!errno) {
    errno = ECONNREFUSED;
  }
  perror("Could not handshake");
  exit(EXIT_FAILURE);
}

void Acpt3WayHandshake(int desc) {
  DatagramHeader synAck = {0};
  synAck.flags = SYN | ACK;
  if (send(desc, &synAck, sizeof(DatagramHeader), NO_FLAGS) == ERROR) {
    perror("Could not handshake");
    exit(EXIT_FAILURE);
  }
}

void Rfse3WayHandshake(int desc) {
  Datagram rst = {{0}};
  rst.header.flags = RST;
  if (send(desc, &rst, DGRAMSIZE(rst), NO_FLAGS) == ERROR) {
    perror("Could not handshake");
    exit(EXIT_FAILURE);
  }
}

EConStatus acceptDatagram(int desc, EConStatus status, ProcessDatagram success) {
  disconnectSocket(desc); //Receive from everyone
  Datagram dgram = receiveDatagram(desc); //Respond only to sender

  if (status == CONNECTED) {
    if (dgram.header.flags & SYN) {
      Rfse3WayHandshake(desc);
      status = NOT_CONNECTED;
    } else if (dgram.header.flags & RST) {
      status = NOT_CONNECTED;
    } else {
      writeData(dgram);
    }
  } else {
    if (dgram.header.flags & SYN) {
      Acpt3WayHandshake(desc);
      status = CONNECTED;
    } else {
      resetConnection(desc);
    }
  }

  return status;
}
