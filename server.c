#include "io.h"
#include "datagram.h"
#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

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

int main(const int argc, const char *argv[]) {
  Address addr = getArguments(argc, argv);
  int desc = createSocket();
  bindSocket(desc, addr);

  EConStatus status = NOT_CONNECTED;
  while (true) {
    status = acceptDatagram(desc, status, setDataPath, writeData);
  }

  close(desc);
  return EXIT_SUCCESS;
}
