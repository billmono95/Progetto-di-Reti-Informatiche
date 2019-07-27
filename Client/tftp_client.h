#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 512
#define PACKET_SIZE 516
#define CMD_SIZE 50

void initMessage(int, const char*, int, struct sockaddr_in);
void help();
void get(int, char*, char*, char*, struct sockaddr_in);
void mode(char*, char*);
void quit();

int RRQ(char* buffer, char* file_name,char* transfermode){
    
    int posizione = 0;
    uint16_t filelen = strlen(file_name);
    uint16_t opcode = htons(1); 
	memcpy(buffer, (uint16_t*)&opcode, 2);
	posizione += 2;

	strcpy(buffer + posizione, file_name);
	posizione += filelen + 1; 

	strcpy(buffer+posizione, transfermode);
	posizione += strlen(transfermode)+1;

    return posizione;

	}

int acksend(char *ack,uint16_t block){


	int position = 0;
		memset(ack, 0, BUFFER_SIZE);
		uint16_t opcode = htons(4);
		memcpy(ack, (uint16_t*)&opcode, 2);
		position += 2;
       
		memcpy(ack + position, (uint16_t*)&block, 2);
		position += 2;
		 return position;

}		