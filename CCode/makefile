SOURCES = entity.c room.c server.c
OBJECTS = entity.o room.o server.o
HEADERS = entity.h room.h
CC = gcc
CFLAGS = 

bruh: $(OBJECTS)
	$(CC) $(SOURCES) $(CFLAGS) -o battleroom
	$(CC) client.c -o client
clean:
	rm -fR *o battleroom client
