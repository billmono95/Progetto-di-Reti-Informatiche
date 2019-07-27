#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 512
#define PACKET_SIZE 516
#define REQ_SIZE 516
#define ACK_SIZE 4
#define IP_SIZE 16

int sendbin(uint16_t block_num,char *pacchetto,unsigned char *buffer_bin, FILE *fp,unsigned int dim_pckt){

                 memset(pacchetto, 0, BUFFER_SIZE); 
                
                 uint16_t opcode = htons(3);
                 
                 int position = 0;
                
                  memset(buffer_bin, 0, FILE_BUFFER_SIZE);

                 fread(buffer_bin, dim_pckt, 1, fp);

                 memcpy(pacchetto + position, (uint16_t*)&opcode, 2);
                 position += 2;
                 memcpy(pacchetto + position, (uint16_t*)&block_num, 2);
                 position += 2;
                 memcpy(pacchetto + position, buffer_bin,dim_pckt);
                 position += dim_pckt;
                 return position;

}


int sendtext(uint16_t block_num,char *pacchetto,char *buffer_file, FILE *fp,unsigned int dim_pckt){
   
                 memset(pacchetto, 0, BUFFER_SIZE);
                 uint16_t opcode = htons(3);
                 int  position = 0;
                 memset(buffer_file, 0, FILE_BUFFER_SIZE);

                 fread(buffer_file, dim_pckt, 1, fp);

                 memcpy(pacchetto + position, (uint16_t*)&opcode, 2);
                 position += 2;
                 memcpy(pacchetto + position, (uint16_t*)&block_num, 2);
                 position += 2;
                 strcpy(pacchetto + position, buffer_file);
                 position += dim_pckt;
                return position; 

  }


 int senderror(uint16_t errorCode,char* bufferError, char* file_name, char *message){

              int  position = 0;
              
              memset(bufferError, 0, BUFFER_SIZE);

              uint16_t opcode = htons(5);
        
              memcpy(bufferError, (uint16_t*)&opcode, 2);
              position += 2;
              memcpy(bufferError+position, (uint16_t*)&errorCode, 2);
              position += 2;
        
              strcpy(bufferError+position, message);
              position += strlen(message)+1;
              

        return position;
 }             