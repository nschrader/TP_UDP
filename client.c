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
  const char* filename;
} Arguments;

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
  openInputFile(arguments.filename);
  int desc = createSocket();
  connectSocket(desc, arguments.address);
  initConnection(desc, arguments.filename);

  uint32_t sequence = 0;
  while (!eofInputFile()) {
    Datagram dgram = readInputData();
    sequence += dgram.header.dataSize;
    dgram.header.flags = NONE;
    dgram.header.sequence = sequence;
    dgram.header.acknowledgment = 0;
    sendDatagram(desc, dgram);
  }

  tmntConnection(desc);
  close(desc);
  closeInputFile();
  return EXIT_SUCCESS;
}
