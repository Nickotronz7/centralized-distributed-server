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
#define BUFFERT 512
// Tamaño de la cola de clientes
#define BACKLOG 100

/* Declaración de funciones */
int duration(struct timeval *start, struct timeval *stop, struct timeval *delta);
int create_server_socket(int port);
void *socketHandler(void *lp);

struct sockaddr_in sock_serv, sock_clt;

int main(int argc, char **argv)
{
    int sfd, fd;
    unsigned int length = sizeof(struct sockaddr_in);
    unsigned int nsid;
    pthread_t thread_id = 0;

    if (argc != 2)
    {
        perror("uso ./server.out <num_port> \n");
        exit(3);
    }

    sfd = create_server_socket(atoi(argv[1]));

    // Función que llega a la función de conexión del cliente
    while (1)
    {
        listen(sfd, BACKLOG);

        nsid = accept(sfd, (struct sockaddr *)&sock_clt, &length);
        if (nsid == -1)
        {
            perror("accept fail");
            return EXIT_FAILURE;
        }
        else
        {
            printf("---------------------\nConexion recibida de  %s\n", inet_ntoa(sock_clt.sin_addr));
            unsigned int *nsid_p;
            *nsid_p = nsid;
            pthread_create(&thread_id, 0, &socketHandler, (void *)nsid_p);
            pthread_detach(thread_id);
        }
    }
}

/* Función que permite la creación de un socket y su conexión al sistema
 * Devuelve un descriptor de archivo en la tabla de descriptores de proceso.
 * bind permite su definición por parte del sistema
 */
int create_server_socket(int port)
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
    bzero(&sock_serv, l);

    sock_serv.sin_family = AF_INET;
    sock_serv.sin_port = htons(port);
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);

    // Asignar una identidad al socket
    if (bind(sfd, (struct sockaddr *)&sock_serv, l) == -1)
    {
        perror("bind fail");
        return EXIT_FAILURE;
    }

    return sfd;
}

void *socketHandler(void *lp)
{
    unsigned int nsid = *(unsigned int *)lp;
    char filename[256], dst[INET_ADDRSTRLEN], buffer[BUFFERT], headers[BUFFERT];
    int fd, fd2;
    long int n, m, count = 0;

    bzero(buffer, BUFFERT);

    if (inet_ntop(AF_INET, &sock_clt.sin_addr, dst, INET_ADDRSTRLEN) == NULL)
    {
        perror("erreur socket");
        exit(4);
    }
    int clt_port = ntohs(sock_clt.sin_port);
    printf("Inicio de conexión para: %s:%d\n", dst, clt_port);

    // Procesar el nombre del archivo con la fecha
    bzero(filename, 256);
    time_t intps = time(NULL);
    struct tm *tmi;
    tmi = localtime(&intps);
    bzero(filename, 256);
    sprintf(filename, "clt.%d.%d.%d.%d.%d.%d", tmi->tm_mday, tmi->tm_mon + 1, 1900 + tmi->tm_year, tmi->tm_hour, tmi->tm_min, tmi->tm_sec);
    printf("Creando el archivo de salida copiado: %s\n", filename);

    bzero(buffer, BUFFERT);
    void *file_mem = malloc(BUFFERT);

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
    char *key = strtok(headers, ";");
    char *name = strtok(NULL, ";");
    char new_name[128] = "new_";
    strcat(new_name, name);

    if ((fd2 = open(new_name, O_CREAT | O_WRONLY, 0600)) == -1)
    {
        perror("open fail");
        exit(3);
    }
    if ((m = write(fd2, file_mem, count)) == -1)
    {
        perror("write fail");
        exit(6);
    }

    return 0;
}
