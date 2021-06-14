#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

/* Tamaño del búfer utilizado para enviar el archivo
 * en varios bloques
 */
#define BUFFERT 512 // Tamaño de la cola de clientes
#define BACKLOG 100
#define QUEUE_SIZE 100
#define MAX_NAME_LENGTH 256

////////////////////////////////////////////////////////////////
///////////////////////STRUCTS/////////////////////////////////

struct image
{
    char name[MAX_NAME_LENGTH];
    int key;
    void *image_mem;
} image;

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

////////////////////////////////////////////////////////////////
///////////////////////FUNCTIONS/////////////////////////////////
/* Declaración de funciones */

int duration(struct timeval *start, struct timeval *stop, struct timeval *delta);
int create_server_socket(int port, struct sockaddr_in *sock_serv);
void *readImageFromSocket(void *lp);
void *listen_to_port(void *params);
void to_file(int count, void *headers, void *file_mem);
void *readMessagesFromSocket(void *params);

////////////////////////////////////////////////////////////////
//////////////////////////GLOBALS////////////////////////////////

struct image images[QUEUE_SIZE];
pthread_mutex_t lock;
int images_head;
int images_tail;

//////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    time_t t;
    pthread_t thread_1, thread_2 = 0;

    srand((unsigned)time(&t));

    if (argc != 2)
    {
        perror("uso ./server.out <num_port> \n");
        exit(3);
    }

    struct socketParameters socket_parameters;
    socket_parameters.port = 2222;
    socket_parameters.handler_function = readImageFromSocket;
    struct socketParameters socket_parameters2;
    socket_parameters2.port = 2223;
    socket_parameters2.handler_function = readMessagesFromSocket;

    pthread_create(&thread_1, 0, &listen_to_port, &socket_parameters);
    pthread_create(&thread_2, 0, &listen_to_port, &socket_parameters2);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
}

void *readMessagesFromSocket(void *params)
{
    struct functionParameters *function_parameters = params;
    struct sockaddr_in *sock_clt = function_parameters->sock_clt;
    unsigned int nsid = function_parameters->nsid_p;

    char dst[INET_ADDRSTRLEN], headers[BUFFERT];
    int clt_port = ntohs(sock_clt->sin_port);

    if (inet_ntop(AF_INET, &sock_clt->sin_addr, dst, INET_ADDRSTRLEN) == NULL)
    {
        perror("erreur socket");
        exit(4);
    }

    printf("Inicio de conexión para: %s:%d\n", dst, clt_port);

    bzero(headers, BUFFERT);
    recv(nsid, headers, BUFFERT, 0);

    char *id = strtok(headers, ";");
    char *ip = strtok(NULL, ";");
    char *port = strtok(NULL, ";");
    char *message = strtok(NULL, ";");
    printf("Raw info is  %s \n", headers);
    printf("El id es %s la ip es %s, en el puerto %s, le mensaje %s", id, ip, port, message);

    puts("\n");

    return 0;
}
void *listen_to_port(void *params)
{
    struct sockaddr_in sock_serv, sock_clt;

    struct socketParameters *socket_parameters = params;
    unsigned int length = sizeof(struct sockaddr_in);
    unsigned int nsid;
    pthread_t thread_id = 0;
    int sfd, fd;
    sfd = create_server_socket(socket_parameters->port, &sock_serv);
    while (1)
    {
        listen(sfd, BACKLOG);
        nsid = accept(sfd, (struct sockaddr *)&sock_clt, &length);
        if (nsid == -1)
        {
            perror("accept fail");
            break;
        }
        else
        {
            puts("Else\n");
            printf("---------------------\nConexion recibida de  %s\n", inet_ntoa(sock_clt.sin_addr));
            struct functionParameters function_parameters;
            function_parameters.nsid_p = nsid;
            function_parameters.sock_clt = &sock_clt;

            pthread_create(&thread_id, 0, socket_parameters->handler_function, (void *)&function_parameters);
            pthread_detach(thread_id);
        }
    }

    return 0;
}

/* Función que permite la creación de un socket y su conexión al sistema
 * Devuelve un descriptor de archivo en la tabla de descriptores de proceso.
 * bind permite su definición por parte del sistema
 */
int create_server_socket(int port, struct sockaddr_in *sock_serv)
{
    int l;
    int sfd;
    int yes = 1;

    sfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket fail");
        return EXIT_SUCCESS;
    }
    /* SOL_SOCKET: Para manipular opciones en el nivel de API de sockets
   * SO_REUSEADDR: Cuando tenga que reiniciar un servidor después de un apagado repentino, esto se puede utilizar
   * no tener un error al crear el socket (la pila de IP del sistema no tenía siempre tuve tiempo para limpiar).
   * Caso donde varios servidores escuchan el mismo puerto ... (?)
   */
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt erreur");
        exit(5);
    }

    // prepara la dirección del socket de destino
    l = sizeof(struct sockaddr_in);
    bzero(sock_serv, l);

    sock_serv->sin_family = AF_INET;
    sock_serv->sin_port = htons(port);
    sock_serv->sin_addr.s_addr = htonl(INADDR_ANY);

    // Asignar una identidad al socket
    if (bind(sfd, (struct sockaddr *)sock_serv, l) == -1)
    {
        perror("bind fail");
        return EXIT_FAILURE;
    }

    return sfd;
}

void *readImageFromSocket(void *params)
{
    struct functionParameters *function_parameters = params;
    struct sockaddr_in *sock_clt = function_parameters->sock_clt;

    unsigned int nsid = function_parameters->nsid_p;
    char dst[INET_ADDRSTRLEN], buffer[BUFFERT], headers[BUFFERT];
    long int n, m, count = 0;
    void *file_mem = malloc(BUFFERT);

    bzero(buffer, BUFFERT);

    if (inet_ntop(AF_INET, &sock_clt->sin_addr, dst, INET_ADDRSTRLEN) == NULL)
    {
        perror("erreur socket");
        exit(4);
    }
    int clt_port = ntohs(sock_clt->sin_port);
    printf("Inicio de conexión para: %s:%d\n", dst, clt_port);

    bzero(buffer, BUFFERT);

    n = recv(nsid, headers, BUFFERT, 0);

    bzero(buffer, BUFFERT);
    n = recv(nsid, buffer, BUFFERT, 0);

    printf("Recived %ld bytes of data", n);
    puts("\n");
    while (n)
    {
        if (n == -1)
        {
            perror("recv fail");
            exit(5);
        }
        memcpy(file_mem + count, buffer, BUFFERT);

        count = count + n;
        bzero(buffer, BUFFERT);
        n = recv(nsid, buffer, BUFFERT, 0);
        if (n)
        {
            void *new_mem_file = realloc(file_mem, count + BUFFERT);
            file_mem = new_mem_file;
        }
        printf("Recived %ld bytes of data", n);
        puts("\n");
    }
    printf("Total %ld bytes of data \n", count);
    printf("wrote data %ld times\n", count / 512);
    printf("El valor que entró es %s", headers);
    puts("\n");
    to_file(count, headers, file_mem);

    return file_mem;
}
void to_file(int count, void *headers, void *file_mem)
{
    int fd, m;
    char *key = strtok(headers, ";");
    char *name = strtok(NULL, ";");

    char random_number[5];
    sprintf(random_number, "%d", 1000 + (rand() % 9000));

    char new_name[MAX_NAME_LENGTH] = "new_"; //Agregar un random al nombre
    strcat(new_name, random_number);

    strcat(new_name, name);

    if ((fd = open(new_name, O_CREAT | O_WRONLY, 0600)) == -1)
    {
        perror("open fail");
        exit(3);
    }

    if ((m = write(fd, file_mem, count)) == -1)
    {
        perror("write fail");
        exit(6);
    }

    pthread_mutex_lock(&lock);
    strcpy(images[images_head].name, name);
    images[images_head].key = atoi(key);
    images[images_head].image_mem = file_mem;
    images_head++;

    if (images_head >= QUEUE_SIZE)
    {
        images_head = 0;
    }
    pthread_mutex_unlock(&lock);
}