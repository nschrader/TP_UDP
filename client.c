#include "io.h"
#include "datagram.h"
#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
  Address address;
  char* filename;
}

static Arguments getArguments(const int argc, const char *argv[]) {
  Arguments arguments;

  if (argc != 4) {
    fprintf(stderr, "Wrong number of arguments\n Usage: %s [ip] [port] [file]\n", argv[0]);
    exit(EXIT_FAILURE);
  } else {
    arguments.address.sin_family = AF_INET;
    if (inet_aton(argv[1], &arguments.address.sin_addr) == 0) {
      fprintf(stderr, "Wrong IPv4 format\n");
      exit(EXIT_FAILURE);
    }

    arguments.address.sin_port = htons(atoi(argv[2]));
    if (arguments.address.sin_port < ROOT_PORTS) {
      fprintf(stderr, "Invalid port\n");
      exit(EXIT_FAILURE);
    }

    arguments.filename = argv[3];
  }

  return arguments;
}

int main(const int argc, const char *argv[]) {
  Arguments arguments = getArguments(argc, argv);
  FILE* file = openFile(arguments.filename);
  int desc = createSocket();
  connectSocket(desc, arguments.address);
  initConnection(desc);

  while (!feof(stdin)) {
    Datagram dgram = readData(file);
    dgram.header.flags = NONE;
    dgram.header.segment = 0;
    dgram.header.acknowledgment = 0;
    sendDatagram(desc, dgram);
  }

  tmntConnection(desc);
  close(desc);
  return EXIT_SUCCESS;
}
