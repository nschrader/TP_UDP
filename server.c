#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ROOT_PORTS  1024

EConStatus status = NOT_CONNECTED;

Address getArguments(const int argc, const char *argv[]) {
  Address addr;

  if (argc != 2) {
    fprintf(stderr, "Wrong number of arguments\n Usage: %s [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  } else {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (addr.sin_port < ROOT_PORTS) {
      fprintf(stderr, "Invalid port\n");
      exit(EXIT_FAILURE);
    }
  }

  return addr;
}

static void handleStateLogic(Datagram dgram, int desc) {
  if (status == CONNECTED) {
    if (dgram.flags & SYN) {
      Rfse3WayHandshake(desc);
      status = NOT_CONNECTED;
    } else if (dgram.flags & RST) {
      status = NOT_CONNECTED;
    } else {
      printf("%s", dgram.data);
    }
  } else {
    if (dgram.flags & SYN) {
      Acpt3WayHandshake(desc);
      status = CONNECTED;
    } else {
      ResetSocket(desc);
    }
  }
}

int main(const int argc, const char *argv[]) {
  Address addr = getArguments(argc, argv);
  int desc = createSocket();
  bindSocket(desc, addr);

  while (true) {
    disconnectSocket(desc);
    Datagram dgram = receiveDatagram(desc);
    handleStateLogic(dgram, desc);
  }

  close(desc);
  return EXIT_SUCCESS;
}
