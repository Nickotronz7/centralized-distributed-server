#include "server.h"

void server(int host_port)
{
    pthread_t thread_id = 0;

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if (hsock == -1)
    {
        printf("Error iniciando el socket %d\n", errno);
        exit(1);
    }

    p_int = (int *)malloc(sizeof(int));
    *p_int = 1;

    if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char *)p_int,
                    sizeof(int)) == -1) ||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char *)p_int,
                    sizeof(int)) == -1))
    {
        printf("Error al setear las opciones %d\n", errno);
        free(p_int);
        exit(1);
    }
    free(p_int);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(hsock, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        fprintf(stderr, "Error uniendo al socket, asegurece de que el puerto esta libre %d\n", errno);
        exit(1);
    }
    if (listen(hsock, 10) == -1)
    {
        fprintf(stderr, "Error al escuchar %d\n", errno);
        exit(1);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);

    while (1)
    {
        printf("Esperando una connecion escuchando %i port\n", host_port);
        csock = (int *)malloc(sizeof(int));
        if ((*csock = accept(hsock, (struct sockaddr *)&sadr, &addr_size)) != -1)
        {
            printf("---------------------\nConexion recivida de  %s\n", inet_ntoa(sadr.sin_addr));
            pthread_create(&thread_id, 0, &socketHandler, (void *)csock);
            pthread_detach(thread_id);
        }
        else
        {
            fprintf(stderr, "Error al aceptar conexion %d\n", errno);
        }
    }
}

void *socketHandler(void *lp)
{
    int *csock = (int *)lp;
    int bytecount;

    read(csock, &buffer_len, sizeof(int));
    printf("%d\n", buffer_len);
    buffer_len =620290;

    char buffer[buffer_len];
    read(csock, buffer, buffer_len);

    printf("%s\n", buffer);

    FILE *fp = fopen("c1.jpg", "w");
    fwrite(buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    char response[] = "200 OK";

    if ((bytecount = send(*csock, response, strlen(response), 0)) == -1)
    {
        fprintf(stderr, "Error al responder data %d\n", errno);
        exit(1);
    }

    printf("Sent bytes %d\n", bytecount);
}