#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#define BUFFERT 512
int duration(struct timeval *start, struct timeval *stop, struct timeval *delta);
int create_client_socket(int port, char *ipaddr);
void *sendOP(void *v_arguments);
struct sockaddr_in sock_serv;

struct sendOP_args
{
    off_t m, count, sz;
    int sfd, fd, l;
    char key[BUFFERT], buf[BUFFERT];
    long int n;
    struct timeval start, stop, delta;
};

#define MAX_NAME_LENGTH 256

struct node
{
    int id;
    int port;
    int working_in;
    char ipaddr[MAX_NAME_LENGTH];

} node;

struct image
{
    char name[MAX_NAME_LENGTH];
    int key;
    int size;
    void *image_mem;
} image;