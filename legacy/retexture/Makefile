CC = gcc
CFLAGS = -Wall -O3 -g
INCLUDES = -I /opt/local/include/
LFLAGS = -L /opt/local/lib/
LIBS = -ljpeg  -lm 

OBJS = retexture.o misc.o bitmaplib.o

all: retexture

retexture: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o retexture $(OBJS) $(LFLAGS) $(LIBS)

retexture.o: retexture.c retexture.h 
	$(CC) $(INCLUDES) $(CFLAGS) -c retexture.c
 
misc.o: misc.c retexture.h
	$(CC) $(INCLUDES) $(CFLAGS) -c misc.c

bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core retexture $(OBJS)

