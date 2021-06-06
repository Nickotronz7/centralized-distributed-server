TARGET = main

CC=gcc

DEBUG=-g

OPT=-O0

WARN=-Wall

CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe

GTKLIB= #`pkg-config --cflags --libs gtk+-3.0`

LD=gcc

LDFLAGS=$(PTHREAD) $(GTKLIB) -export-dynamic -lssl -lcrypto

OBJS=main.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: main.c
	$(CC) -c $(CCFLAGS) main.c $(GTKLIB) -o main.o

clean:
	rm -f *.o $(TARGET)
