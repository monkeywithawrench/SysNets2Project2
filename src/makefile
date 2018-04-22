CC = gcc
CFLAGS = -g -Wall

objects = bbserver.o bbclient.o

all: bbserver bbclient

bbserver: bbserver.c
	$(CC) -o bbserver bbserver.c bbutils.h $(CFLAGS)

bbclient: bbclient.c
	$(CC) -o bbclient bbclient.c bbutils.h $(CFLAGS)

.PHONY : clean
clean:
	rm $(all) $(objects)
