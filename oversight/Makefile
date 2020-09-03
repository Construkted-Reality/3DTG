CC = gcc
CFLAGS = -Wall -O3
INCLUDES = -I /opt/local/include
LFLAGS = -L /opt/local/lib
LIBS = -ljpeg -lm 

OBJS = oversight.o bitmaplib.o 

all: oversight

oversight: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o oversight $(OBJS) $(LFLAGS) $(LIBS)

oversight.o: oversight.c oversight.h
	$(CC) $(INCLUDES) $(CFLAGS) -c oversight.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core oversight $(OBJS)

