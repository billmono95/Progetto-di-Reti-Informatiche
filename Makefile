PATH_CLIENT=./Client/
PATH_SERVER=./Server/

CC=gcc
CFLAGS=-Wall

all: $(PATH_CLIENT)tftp_client.o $(PATH_SERVER)tftp_server.o
		$(CC) $(PATH_CLIENT)tftp_client.o -o $(PATH_CLIENT)tftp_client
		$(CC) $(PATH_SERVER)tftp_server.o -o $(PATH_SERVER)tftp_server

tftp_client: tftp_client.o
		$(CC) $(PATH_CLIENT)tftp_client.o -o $(PATH_CLIENT)tftp_client

tftp_client.o: $(PATH_CLIENT)tftp_client.c $(PATH_CLIENT)tftp_client.h
		$(CC) $(CFLAGS)  -c $(PATH_CLIENT)tftp_client.c -o $(PATH_CLIENT)tftp_client.o

tftp_server: tftp_server.o
		$(CC)  $(PATH_SERVER)tftp_server.o -o $(PATH_SERVER)tftp_server

tftp_server.o: $(PATH_SERVER)tftp_server.c $(PATH_SERVER)tftp_server.h
		$(CC) $(CFLAGS)  -c $(PATH_SERVER)tftp_server.c -o $(PATH_SERVER)tftp_server.o

clean:
	rm $(PATH_CLIENT)*.o $(PATH_CLIENT)tftp_client
	rm $(PATH_SERVER)*.o $(PATH_SERVER)tftp_server
