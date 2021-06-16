#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <semaphore.h>
#define BUFFERT 512
#define MAX_NAME_LENGTH 256
/* Tamaño del búfer utilizado para enviar el archivo
 * en varios bloques
 */
#define BACKLOG 100
#define QUEUE_SIZE 100
#define MAX_NODES 5

////////////////////////////////////////////////////////////////
///////////////////////FUNCTIONS/////////////////////////////////
/* Declaración de funciones */

int duration(struct timeval *start, struct timeval *stop, struct timeval *delta);
int create_server_socket(int port, struct sockaddr_in *sock_serv);
void *readImageFromSocket(void *lp);
void *listen_to_port(void *params);
void processImageInNode(int count, void *headers, void *file_mem);
void *readMessagesFromSocket(void *params);
int duration(struct timeval *start, struct timeval *stop, struct timeval *delta);
int create_client_socket(int port, char *ipaddr);
void *sendOP(void *v_arguments);
void *waiting(void *node_index);
void increaseHead();

struct sockaddr_in sock_serv;
////////////////////////////////////////////////////////////////
///////////////////////STRUCTS/////////////////////////////////
struct sendOP_args
{
    off_t m, count, sz;
    int sfd, fd, l;
    char key[BUFFERT], buf[BUFFERT];
    long int n;
    struct timeval start, stop, delta;
};

struct socketParameters
{
    void *handler_function;
    int port;

} socket_parameters;

struct functionParameters
{
    unsigned int nsid_p;
    struct sockaddr_in *sock_clt;
} functionParameters;

struct node
{
    int id;
    int port;
    int working_in;
    char ipaddr[MAX_NAME_LENGTH];
    sem_t semaphore;

} node;

struct image
{
    char name[MAX_NAME_LENGTH];
    int key;
    int size;
    void *image_mem;
} image;
////////////////////////////////////////////////////////////////
//////////////////////////GLOBALS////////////////////////////////

struct image images[QUEUE_SIZE];
struct node nodes[MAX_NODES];
pthread_mutex_t lock;
int images_head;
int images_tail;
pthread_mutex_t head_mutex;
