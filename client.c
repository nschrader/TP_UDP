#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ROOT_PORTS  1024

static Address getArguments(const int argc, const char *argv[]) {
  Address addr;

  if (argc != 3) {
    fprintf(stderr, "Wrong number of arguments\n Usage: %s [ip] [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  } else {
    addr.sin_family = AF_INET;
    if (inet_aton(argv[1], &addr.sin_addr) == 0) {
      fprintf(stderr, "Wrong IPv4 format\n");
      exit(EXIT_FAILURE);
    }

    addr.sin_port = htons(atoi(argv[2]));
    if (addr.sin_port < ROOT_PORTS) {
      fprintf(stderr, "Invalid port\n");
      exit(EXIT_FAILURE);
    }
  }

  return addr;
}

int main(const int argc, const char *argv[]) {
  Address addr = getArguments(argc, argv);
  int desc = createSocket();
  connectSocket(desc, addr);
  Init3WayHandshake(desc);

  while(!feof(stdin)) {
    Datagram dgram = readData();
    dgram.header.flags = 0;
    dgram.header.segment = 0;
    dgram.header.acknowledgment = 0;
    sendDatagram(desc, dgram);
  }

  close(desc);
  return EXIT_SUCCESS;
}
