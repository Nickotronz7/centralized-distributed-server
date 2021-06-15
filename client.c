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

#define BUFFERT 512

int duration(struct timeval *start, struct timeval *stop, struct timeval *delta);
int create_client_socket(int port, char *ipaddr);

struct sockaddr_in sock_serv;

int comunicate(char *ip, char *portself, char *ipself, char *texto, char *id)
{
	time_t t;
	srand((unsigned)time(&t));
	struct timeval start, stop, delta;
	int sfd;
	off_t count = 0, m, sz;
	long int n;
	int l = sizeof(struct sockaddr_in);

	sfd = create_client_socket(2223, ip);

	if (connect(sfd, (struct sockaddr *)&sock_serv, l) == -1)
	{
		perror("conexion error\n");
		exit(3);
	}
	puts("hola");
	char key[BUFFERT];
	strcat(key, id);
	strcat(key, ";");
	strcat(key, ipself);
	strcat(key, ";");
	puts(portself);
	strcat(key, portself);
	strcat(key, ";");
	strcat(key, texto);
	m = send(sfd, key, BUFFERT, 0);

	puts("\n");
	// lectura acaba de devolver 0: final del archivo

	// para desbloquear el serv
	bzero(key, BUFFERT);
	//m = send(sfd, key, 0, 0);
	gettimeofday(&stop, NULL);
	duration(&start, &stop, &delta);

	printf("Número de bytes transferidos: %ld\n", count);
	printf("En un tamaño total: %ld \n", sz);
	printf("Por una duración total de: %ld.%ld \n", delta.tv_sec, delta.tv_usec);

	close(sfd);
	return EXIT_SUCCESS;
}

// Función que permite calcular la duración del envío
int duration(struct timeval *start, struct timeval *stop, struct timeval *delta)
{
	suseconds_t microstart, microstop, microdelta;

	microstart = (suseconds_t)(100000 * (start->tv_sec)) + start->tv_usec;
	microstop = (suseconds_t)(100000 * (stop->tv_sec)) + stop->tv_usec;
	microdelta = microstop - microstart;

	delta->tv_usec = microdelta % 100000;
	delta->tv_sec = (time_t)(microdelta / 100000);

	if ((*delta).tv_sec < 0 || (*delta).tv_usec < 0)
		return -1;
	else
		return 0;
}

/*
 * Función que permite la creación de un socket
 * Devuelve un descriptor de archivo
 */
int create_client_socket(int port, char *ipaddr)
{
	int l;
	int sfd;

	sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
	{
		perror("socket fail");
		return EXIT_FAILURE;
	}

	// prepara la dirección del socket de destino
	l = sizeof(struct sockaddr_in);
	bzero(&sock_serv, l);

	sock_serv.sin_family = AF_INET;
	sock_serv.sin_port = htons(port);
	if (inet_pton(AF_INET, ipaddr, &sock_serv.sin_addr) == 0)
	{
		printf("Invalid IP adress\n");
		return EXIT_FAILURE;
	}

	return sfd;
}