#ifndef MISC_H
#define MISC_H

#include <sys/socket.h>
#include <netinet/in.h>

int createSocket();
void connectSocket(int desc, struct sockaddr_in addr);
void bindSocket(int desc, struct sockaddr_in addr);
void recieveSocket(int desc);
void sendSocket(int desc);

#endif
