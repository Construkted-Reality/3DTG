CC = gcc
CFLAGS = -Wall -O3
INCLUDES =
LFLAGS = 
LIBS = -lm 

OBJS = gridsplit.o misc.o

all: gridsplit

gridsplit: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o gridsplit $(OBJS) $(LFLAGS) $(LIBS)

misc.o: misc.c gridsplit.h
	$(CC) $(INCLUDES) $(CFLAGS) -c misc.c

gridsplit.o: gridsplit.c gridsplit.h objlib.c objlib.h misc.c 
	$(CC) $(INCLUDES) $(CFLAGS) -c gridsplit.c
 
clean:
	rm -rf core gridsplit $(OBJS)

