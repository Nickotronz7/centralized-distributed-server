all:
	make server

server: server.c
	gcc -o server  server.c -lpthread

clean:
	rm -f server 
