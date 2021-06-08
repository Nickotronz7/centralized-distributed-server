TARGET = main

CC=gcc

DEBUG=-g

OPT=-O0

WARN=-Wall

CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

LD=gcc

LDFLAGS=$(PTHREAD) $(GTKLIB) -export-dynamic -lm -lpthread

OBJS=main.o client.o server.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CCFLAGS) main.c $(GTKLIB) -o main.o

client.o: client/client.c
	$(CC) -c $(CCFLAGS) client/client.c -o client.o

server.o: server/server.c
	$(CC) -c $(CCFLAGS) server/server.c -o server.o

clean:
	rm -f *.o $(TARGET)
