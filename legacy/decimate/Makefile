CC = gcc
CFLAGS = -Wall -O3
INCLUDES =
LFLAGS = 
LIBS = -lm 

OBJS = decimate.o

all: decimate

decimate: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o decimate $(OBJS) $(LFLAGS) $(LIBS)

decimate.o: decimate.c decimate.h
	$(CC) $(INCLUDES) $(CFLAGS) -c decimate.c
 
clean:
	rm -rf core decimate $(OBJS)

