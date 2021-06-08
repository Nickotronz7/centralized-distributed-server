#ifndef SERVER_CLIENT_SERVER_H
#define SERVER_CLIENT_SERVER_H

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <malloc.h>

void server(int host_port);

void *socketHandler(void *lp);

int hsock, *p_int, err, *csock, buffer_len;

struct sockaddr_in my_addr;

struct sockaddr_in sadr;

#endif //SERVER_CLIENT_SERVER_H