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
#include "client.c"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

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
char ipserver[20],ipself[20],port[20],id[20];
char key[BUFFERT];

struct sockaddr_in sock_serv, sock_clt;

int main(int argc, char **argv)
{
    int sfd, fd;
    unsigned int length = sizeof(struct sockaddr_in);
    unsigned int nsid;
    time_t t;
    srand((unsigned)time(&t));
    pthread_t thread_id = 0;
    
    if (argc != 4)
    {
        perror("uso ./procesador.out <num_port> <ip_server> <ip_self>\n");
        exit(3);
    }

    sfd = create_server_socket(atoi(argv[1]));
    strcpy(ipserver,argv[2]);
    strcpy(port, argv[1]); 
    strcpy(ipself,argv[3]);
    sprintf(id, "%d", rand() % 1000000);
    comunicate(ipserver,port,ipself,"join",id);
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
    char dst[INET_ADDRSTRLEN], buffer[BUFFERT], headers[BUFFERT];
    int fd, fd2;
    long int n, m, count = 0;
    pthread_t thread_id = 0;

    bzero(buffer, BUFFERT);

    if (inet_ntop(AF_INET, &sock_clt.sin_addr, dst, INET_ADDRSTRLEN) == NULL)
    {
        perror("erreur socket");
        exit(4);
    }
    int clt_port = ntohs(sock_clt.sin_port);
    printf("Inicio de conexión para: %s:%d\n", dst, clt_port);

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
    printf("wrote data %ld times\n", count / BUFFERT);
    printf("El valor que entró es %s", headers);
    puts("\n");
    char *key = strtok(headers, ";");
    char *name = strtok(NULL, ";");
    char new_name[128] = "";
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
    int xor = atoi(key);
    filtrxor(name, xor);
    return 0;
}

int filtrxor(char* name,int xor) {
    
    char new_name[128] = "xor_";
    strcat(new_name, name);
    puts(name);
     int width, height, channels;
     unsigned char *img = stbi_load(name, &width, &height, &channels, 0);
     if(img == NULL) {
         printf("Error in loading the image\n");
         exit(1);
     }
     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
    size_t img_size = width * height * channels;
    unsigned char *sepia_img = malloc(img_size);
     if(sepia_img == NULL) {
         printf("Unable to allocate memory for the sepia image.\n");
         exit(1);
     }
 
     for(unsigned char *p = img, *pg = sepia_img; p != img + img_size; p += channels, pg += channels) {
         *pg       = (uint8_t)fmin(*p ^ xor , 255.0);         // red
         *(pg + 1) = (uint8_t)fmin(*p ^ xor , 255.0);         // green
         *(pg + 2) = (uint8_t)fmin(*p ^ xor , 255.0);         // blue        
         if(channels == 4) {
             *(pg + 3) = *(p + 3);
         }
     }
     sleep(4);
        
     stbi_write_jpg(new_name, width, height, channels, sepia_img, 100);  
     comunicate(ipserver,port,ipself,"free",id);

}
