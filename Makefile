CC=gcc
CFLAGS=-Wall

all: client server

client: src/client/client.c
	$(CC) $(CFLAGS) src/client/client.c -o build/client

server: src/server/server.c
	$(CC) $(CFLAGS) src/server/server.c -o build/server

clean:
	rm -f build/client build/server
