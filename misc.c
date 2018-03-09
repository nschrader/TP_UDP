#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
