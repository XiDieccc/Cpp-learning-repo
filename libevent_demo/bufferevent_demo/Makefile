all: server client

server: server.c
	gcc server.c -o server -levent

client: client.c
	gcc client.c -o client -levent

clean:
	rm -f client server