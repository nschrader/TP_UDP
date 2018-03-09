#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RCVSIZE     1024
#define ERROR       -1

#define STRSIZE(s) (strlen(s)+1)

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
  char buffer[RCVSIZE];
  recv(desc, buffer, RCVSIZE, 0);
  printf("%s", buffer);
}

void sendSocket(int desc) {
  char msg[RCVSIZE] = {0};
  fgets(msg, RCVSIZE, stdin);
  send(desc, msg, STRSIZE(msg), 0);
}
