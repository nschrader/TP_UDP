#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ROOT_PORTS  1024

struct sockaddr_in getArguments(const int argc, const char *argv[]) {
  struct sockaddr_in addr;

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

int main (const int argc, const char *argv[]) {
  struct sockaddr_in addr = getArguments(argc, argv);
  int desc = createSocket();
  bindSocket(desc, addr);

  while (true) {
    recieveSocket(desc);
  }

  close(desc);
  return EXIT_SUCCESS;
}
