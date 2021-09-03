CC = gcc
CFLAGS = -Wall -O3
INCLUDES =
LFLAGS = 
LIBS = -lm 

OBJS = split.o

all: split

split: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o split $(OBJS) $(LFLAGS) $(LIBS)

split.o: split.c split.h objlib.c objlib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c split.c
 
clean:
	rm -rf core split $(OBJS)

