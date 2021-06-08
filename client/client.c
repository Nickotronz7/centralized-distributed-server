#include "client.h"

void client(char *host_name, int host_port, char *img_path)
{
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if (hsock == -1)
    {
        printf("Error al inicializar el socket %d\n", errno);
        exit(1);
    }

    p_int = (int *)malloc(sizeof(int));
    *p_int = 1;

    if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char *)p_int,
                    sizeof(int)) == -1) ||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char *)p_int,
                    sizeof(int)) == -1))
    {
        printf("Error al establecer las opciones %d\n", errno);
        free(p_int);
        exit(1);
    }

    free(p_int);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name);

    if (connect(hsock, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        if ((err = errno) != EINPROGRESS)
        {
            fprintf(stderr, "Error connecting socket %d\n", errno);
            exit(1);
        }
    }

    FILE *pic;
    pic = fopen(img_path, "r");
    fseek(pic, 0, SEEK_END);
    buffer_len = ftell(pic);
    fseek(pic, 0, 0);
    // printf("%i\n", buffer_len);

    write(socket, &buffer_len, sizeof(buffer_len));

    char send_buf[buffer_len];
    while (!feof(pic))
    {
        fread(send_buf, 1, sizeof(send_buf), pic);
        write(socket, send_buf, sizeof(send_buf));
        bzero(send_buf, sizeof(send_buf));
    }

    if ((bytecount = recv(hsock, send_buf, buffer_len, 0)) == -1)
    {
        fprintf(stderr, "Error receiving data %d\n", errno);
        exit(1);
    }
    printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, send_buf);
    close(hsock);
}
