all:
	gcc -std=c17 -Wall  procesador.c -o procesador -lm -lpthread
	gcc -o client client.c