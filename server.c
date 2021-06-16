
#include "client.c"
#include "logger.c"

//////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    FILE *log_file = fopen(LOG_FILE, "w");
    fclose(log_file);

    time_t t;
    pthread_t thread_1, thread_2 = 0;

    srand((unsigned)time(&t));

    if (argc != 2)
    {
        perror("uso ./server.out <num_port> \n");
        exit(3);
    }

    struct socketParameters socket_parameters;
    socket_parameters.port = atoi(argv[1]);
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

    log_event(NULL, "Inicio de conexión para: %s:%d\n", dst, clt_port);

    bzero(headers, BUFFERT);
    recv(nsid, headers, BUFFERT, 0);

    char *id = strtok(headers, ";");
    char *ip = strtok(NULL, ";");
    char *port = strtok(NULL, ";");
    char *message = strtok(NULL, ";");
    if (strcmp(message, "join") == 0)
    {
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (nodes[i].id == 0)
            {
                nodes[i].id = atoi(id);
                sem_init(&nodes[i].semaphore, 0, 3);
                strcpy(nodes[i].ipaddr, ip);
                nodes[i].port = atoi(port);
                break;
            }
        }
    }
    if (strcmp(message, "free") == 0)
    {
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (nodes[i].id == atoi(id))
            {
                log_event("INFO", "Se liberó un espacio en el nodo %d", nodes[i].id);
                sem_post(&nodes[i].semaphore);

                break;
            }
        }
    }

    log_event(NULL, "Mensaje recibido de %s desde la IP %s utilizando el puerto %s: %s \n", id, ip, port, message);

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
            log_event(NULL, "Escuchando... Conexion recibida de  %s\n", inet_ntoa(sock_clt.sin_addr));
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
    bzero(headers, BUFFERT);

    if (inet_ntop(AF_INET, &sock_clt->sin_addr, dst, INET_ADDRSTRLEN) == NULL)
    {
        perror("erreur socket");
        exit(4);
    }
    int clt_port = ntohs(sock_clt->sin_port);
    log_event(NULL, "Inicio de conexión para: %s:%d\n", dst, clt_port);

    bzero(buffer, BUFFERT);

    n = recv(nsid, headers, BUFFERT, 0);

    bzero(buffer, BUFFERT);
    n = recv(nsid, buffer, BUFFERT, 0);

    while (n)
    {
        if (n == -1)
        {
            log_event(NULL, "%s", "Error in reciving data");
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
        //  printf("Recived %ld bytes of data", n);
        //  puts("\n");
    }
    //  log_event(NULL, "Total %ld bytes of data \n", count);
    // log_event(NULL, "wrote data %ld times\n", count / BUFFERT);
    // log_event(NULL, "El valor que entró es %s", headers);
    processImageInNode(count, headers, file_mem);

    return file_mem;
}
void processImageInNode(int count, void *headers, void *file_mem)
{

    int fd, m;
    char *key = strtok(headers, ";");
    char *name = strtok(NULL, ";");

    char random_number[6];
    sprintf(random_number, "%d", 10000 + (rand() % 90000));

    char new_name[MAX_NAME_LENGTH] = ""; //Agregar un random al nombre
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

    int node_index = rand() % MAX_NODES;

    // esto es busy waiting hay que quitar esta mierda salu3
    while (1)
    {
        if (nodes[node_index].id != 0)
        {
            break;
        }
        node_index = rand() % MAX_NODES;
    }
    pthread_t wait_thread;

    pthread_create(&wait_thread, 0, &waiting, &nodes[node_index].id);
    sem_wait(&nodes[node_index].semaphore);
    pthread_cancel(wait_thread);
    int image_index = images_head;
    increaseHead();

    strcpy(images[image_index].name, new_name);
    images[image_index].key = atoi(key);
    images[image_index].image_mem = file_mem;
    images[image_index].size = count;
    struct image *new_image2 = &images[image_index];

    log_event(NULL, "Se ha asignado el nodo %d a la imagen %s", nodes[node_index].id, images[image_index].name);

    sendToNode(&nodes[node_index], &images[image_index]);
    free(images[image_index].image_mem);
}
void increaseHead()
{
    pthread_mutex_lock(&head_mutex);
    int next_head = images_head + 1;

    if (next_head >= QUEUE_SIZE)
    {
        log_event("WARNING", "La cola está llena imagen rechazada");

        next_head = 0;
    }
    else
    {
        images_head = next_head;
    }
    pthread_mutex_unlock(&head_mutex);
}

void *waiting(void *node_index)
{
    int node = *(int *)node_index;
    sleep(1);
    while (1)
    {
        printf("Esperando por el nodo %d", node);
        puts("\n");
        sleep(1);
    }
}