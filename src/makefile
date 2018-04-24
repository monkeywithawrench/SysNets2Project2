CC = gcc
CFLAGS = -g -Wall

objects = bbserver.o bbclient.o

all: bbserver bbclient


bbserver: bbserver.c bbutils.c bbutils.h
	$(CC) -o bbserver bbserver.c bbutils.c bbutils.h $(CFLAGS)

bbclient: bbclient.c bbutils.c bbutils.h
	$(CC) -o bbclient bbclient.c bbutils.c bbutils.h $(CFLAGS) -lpthread

.PHONY : clean
clean:
	rm $(all) $(objects)
