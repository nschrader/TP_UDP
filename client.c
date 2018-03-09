#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "misc.h"

#define RCVSIZE     1024
#define ROOT_PORTS  1024
#define ERROR       -1

#define STRSIZE(s) (strlen(s)+1)

static struct sockaddr_in getArguments(const int argc, const char *argv[]) {
  struct sockaddr_in addr;

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

static void connectSocket(int desc, struct sockaddr_in addr) {
  if (connect(desc, (struct sockaddr*) &addr, sizeof(addr)) == ERROR) {
    perror("connect failed");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

static void sendSocket(int desc) {
  char msg[RCVSIZE] = {0};
  fgets(msg, RCVSIZE, stdin);
  send(desc, msg, STRSIZE(msg), 0);
}

int main (const int argc, const char *argv[]) {
  struct sockaddr_in addr = getArguments(argc, argv);
  int desc = createSocket();
  connectSocket(desc, addr);
  sendSocket(desc);

  close(desc);
  return EXIT_SUCCESS;
}
