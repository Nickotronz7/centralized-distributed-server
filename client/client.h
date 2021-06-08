#ifndef CLIENT_H
#define CLIENT_H

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <malloc.h>

void client(char *host_name, int host_port, char *img_path);

struct sockaddr_in my_addr;

int bytecount, buffer_len, hsock, *p_int, err;

#endif