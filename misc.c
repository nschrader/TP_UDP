#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR -1
#define NO_FLAGS 0

int createSocket() {
  const int valid = 1;
  int desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (desc < 0) {
    perror("Cannot create socket");
    exit(EXIT_FAILURE);
  }
  setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
  return desc;
}

void connectSocket(int desc, struct sockaddr_in addr) {
  if (connect(desc, (struct sockaddr*) &addr, sizeof(addr)) == ERROR) {
    perror("connect failed");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

void bindSocket(int desc, struct sockaddr_in addr) {
  if (bind(desc, (struct sockaddr*) &addr, sizeof(addr)) == ERROR) {
    perror("Bind fail");
    close(desc);
    exit(EXIT_FAILURE);
  }
}

void recieveSocket(int desc) {
  Datagram dgram = {0};
  recv(desc, &dgram, sizeof(Datagram), NO_FLAGS);
  printf("%s", dgram.data);
}

void sendSocket(int desc) {
  Datagram dgram = {0};
  fgets(dgram.data, SEGSIZE, stdin);
  send(desc, &dgram, DGRAMSIZE(dgram), NO_FLAGS);
}
