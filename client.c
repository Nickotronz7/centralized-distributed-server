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

int main(int argc, char **argv)
{
	time_t t;
	srand((unsigned)time(&t));
	struct timeval start, stop, delta;
	int sfd, fd;
	char buf[BUFFERT];
	off_t count, m, sz;
	long int n;
	int times;
	int l = sizeof(struct sockaddr_in);
	struct stat buffer;
	pthread_t thread_id = 0;

	if (argc < 5)
	{
		//                         1           2         3          4       5
		printf("Error usage : %s <ip_serv> <port_serv> <filename> <times> <xor_key>\n",
			   argv[0]);
		return EXIT_FAILURE;
	}
	times = atoi(argv[4]);

	// printf("")

	while (times > 0)
	{

		sfd = create_client_socket(atoi(argv[2]), argv[1]);
		count = 0;

		if ((fd = open(argv[3], O_RDONLY)) == -1)
		{
			perror("open fail");
			return EXIT_FAILURE;
		}

		if (stat(argv[3], &buffer) == -1)
		{
			perror("stat fail");
			return EXIT_FAILURE;
		}
		else
			sz = buffer.st_size;

		bzero(&buf, BUFFERT);

		if (connect(sfd, (struct sockaddr *)&sock_serv, l) == -1)
		{
			perror("conexion error\n");
			exit(3);
		}
		gettimeofday(&start, NULL);
		n = read(fd, buf, BUFFERT);

		char key[BUFFERT];
		if (argc > 5)
		{
			sprintf(key, "%d", atoi(argv[5]) % 250);
		}
		else
		{
			sprintf(key, "%d", rand() % 250);
		}
		strcat(key, ";");
		strcat(key, argv[3]);
		printf("La llave es %s", key);

		m = send(sfd, key, n, 0);

		puts("\n");
		while (n)
		{
			if (n == -1)
			{
				perror("read fails");
				return EXIT_FAILURE;
			}
			m = send(sfd, buf, n, 0);
			if (m == -1)
			{
				perror("send error");
				return EXIT_FAILURE;
			}
			count += m;
			bzero(buf, BUFFERT);
			n = read(fd, buf, BUFFERT);
		}
		// lectura acaba de devolver 0: final del archivo

		// para desbloquear el serv
		//m = send(sfd, buf, 0, 0);
		gettimeofday(&stop, NULL);
		duration(&start, &stop, &delta);

		printf("Número de bytes transferidos: %ld\n", count);
		printf("En un tamaño total: %ld \n", sz);
		printf("Por una duración total de: %ld.%ld \n", delta.tv_sec,
			   delta.tv_usec);

		close(sfd);
		times -= 1;
		sleep(1);
	}
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

void *sendOP(void *v_arguments)
{

	struct sendOP_args *args = (struct sendOP_args *)v_arguments;

	off_t m = args->m;
	int sfd = args->sfd;
	char key[BUFFERT], buf[BUFFERT];
	strcpy(key, args->key);
	long int n = args->n;
	int l = args->l;
	strcpy(buf, args->buf);
	off_t count = args->count;
	int fd = args->fd;
	struct timeval start, stop, delta;
	start = args->start;
	stop = args->stop;
	delta = args->delta;
	off_t sz = args->sz;

	m = send(sfd, key, n, 0);

	puts("\n");
	while (n)
	{
		if (n == -1)
		{
			perror("read fails");
			exit(-1);
		}
		m = send(sfd, buf, n, 0);
		if (m == -1)
		{
			perror("send error");
			exit(-1);
		}
		count += m;
		bzero(buf, BUFFERT);
		n = read(fd, buf, BUFFERT);
	}
	// lectura acaba de devolver 0: final del archivo

	// para desbloquear el serv
	//m = send(sfd, buf, 0, 0);
	gettimeofday(&stop, NULL);
	duration(&start, &stop, &delta);

	printf("Número de bytes transferidos: %ld\n", count);
	printf("En un tamaño total: %ld \n", sz);
	printf("Por una duración total de: %ld.%ld \n", delta.tv_sec, delta.tv_usec);

	// close(sfd);
}