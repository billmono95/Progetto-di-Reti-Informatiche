#include "tftp_server.h"
int main(int argc, char** argv ){
 
    int ret, sd,newsd,pos,valid;
    
    unsigned int addrlen;
    unsigned int dim_rimasti;
    pid_t pid;
    struct sockaddr_in my_addr, client_addr, new_addr;
    char buffer[BUFFER_SIZE];
    char fileName[BUFFER_SIZE];
    char mode[BUFFER_SIZE];
    char ip_client[IP_SIZE];
    char pacchetto[BUFFER_SIZE];
    char bufferError[BUFFER_SIZE];
    uint16_t blocksucc;
    char buffer_file[FILE_BUFFER_SIZE];
    unsigned char buffer_bin[FILE_BUFFER_SIZE];
    char message[FILE_BUFFER_SIZE];
    FILE* fp;

 if(argc != 3){
        printf("\nPer avviare il programma digita ./tftp_server <porta> <directory files>\n");
        return 0;
    }    

    int port = atoi(argv[1]);
    char* directory = argv[2];

    /* Creazione socket */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
     if(sd < 0){
        printf("Errore in fase di connessione: \n");
        exit(0);
    }
    
    /* Creazione indirizzo di bind */   
    memset(&my_addr, 0, sizeof(my_addr)); //Pulisco la struttura
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    printf("\nSocket listener creato.\n");


    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
   
    if(ret < 0){
        perror("Errore in fase di bind: \n");
        exit(0);
    } else {

      printf("\nAspetto connessioni.\n");
    } while(1) {

                 memset(&buffer,0,BUFFER_SIZE);
                 addrlen = sizeof(client_addr);
                       
                 ret = recvfrom(sd, (void*)buffer, REQ_SIZE, 0,(struct sockaddr*)&client_addr, &addrlen);
                     
                  if(ret < 0){
                    perror("Errore in fase di ricezione: \n");
                    exit(0);
                }else {
                printf("\nRicezione avvenuta\n");
                }         
           
           pid = fork();
           if (pid == -1){

              printf("Errore sulla fork\n");
              exit(-1);
           }
        

        if( pid == 0 ){

          newsd =  socket(AF_INET, SOCK_DGRAM, 0);
              memset(&new_addr, 0, sizeof(new_addr)); //Pulisco la struttura
             new_addr.sin_family = AF_INET;
             new_addr.sin_port = htons(0);
             new_addr.sin_addr.s_addr = INADDR_ANY;
           
    ret = bind(newsd, (struct sockaddr*)&new_addr, sizeof(new_addr) );
   
    if(ret < 0){
        perror("Errore in fase di bind: \n");
        exit(0);
    }
        close(sd);
    
          uint16_t opcode,errorCode;
          memcpy(&opcode, buffer, 2);
          valid = 0;
          opcode = ntohs(opcode);

        if(opcode == 1)  {
                   
            
        
            memset(fileName, 0, BUFFER_SIZE);
            strcpy(fileName, buffer+2);
            strcpy(mode, buffer + (int)strlen(fileName) + 3); //3= 2byte per opcode + 1 byte carattere di fine stringa

            memset(ip_client, 0, IP_SIZE);
            inet_ntop(AF_INET, &client_addr, ip_client, IP_SIZE);

            printf("\nRichiesta di download del file %s in modalità %s da %s\n", fileName, mode, ip_client);
        
            char* path = malloc(strlen(directory)+strlen(fileName)+2);
            strcpy(path, directory);
            strcat(path, "/");
            strcat(path, fileName);
            
            if(!strcmp(mode, "netascii\0"))
              fp = fopen(path, "r");
            else
              fp = fopen(path, "rb");

            free(path);

            if(fp == NULL){
            errorCode = htons(1);
            memset(message, 0, FILE_BUFFER_SIZE);

            strcpy(message, "File non trovato\0");
            pos = senderror(errorCode, bufferError, fileName,message);
            newsd =  socket(AF_INET, SOCK_DGRAM, 0);

            printf("\nERRORE! Lettura del file %s non riuscita\n", fileName);

              ret = sendto(newsd, bufferError, pos, 0,(struct sockaddr*)&client_addr, sizeof(client_addr));

              if(ret < 0){
                perror("\n[ERRORE]: Invio dei dati non riuscito\n");
                exit(0);
              }

              close(newsd);
              continue;
            } else {

              printf("\nLettura del file %s riuscita\n", fileName);
      
              if(!strcmp(mode, "netascii\0")){ 
                // Lettura della lunghezza del contenuto del file 
                unsigned int length = 0;
                while(fgetc(fp) != EOF)
                  length++;           
                //Rimetto l'indicatore di posizione del file all'inizio 
                 fseek(fp, 0 , SEEK_SET);

                 unsigned int dim_pckt = (length > FILE_BUFFER_SIZE)?FILE_BUFFER_SIZE:length;
                 dim_rimasti = length - dim_pckt;
                 uint16_t block_num = htons(1);
                 int pos = sendtext( block_num, pacchetto, buffer_file, fp, dim_pckt);
                 blocksucc = 1;

                ret = sendto(newsd, pacchetto, pos, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));

                if(ret < 0){
                  perror("[ERRORE]: errore durante la send del blocco al client.");
                  exit(0);
                }
              }else { //modalità bin
                          
                fseek(fp, 0 , SEEK_END); //mi sposto in fondo
                //Ritorna la posizione corrente nel file
                unsigned int length = ftell(fp); //guardo in che posizione sto
                //Resetto l'indicatore
                fseek(fp, 0 , SEEK_SET); //ritorno in cima per il prossimo file
                unsigned int dim_pckt = (length > FILE_BUFFER_SIZE)?FILE_BUFFER_SIZE:length;
                dim_rimasti = length - dim_pckt;
                uint16_t block_num = htons(1);
                
                int pos = sendbin( block_num, pacchetto, buffer_bin, fp, dim_pckt);
                   blocksucc = 1;
                 
                 ret = sendto(newsd, pacchetto, pos, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
                
                if(ret < 0) {
                  perror("Errore nella send");
                  exit(0);
                }
            
              }
            }
           valid = 1;
          } if(opcode != 1 || (opcode == 4 && valid == 0)){//se la prima richiesta è diversa da opcode = 1
                errorCode = htons(2);
                memset(message, 0, FILE_BUFFER_SIZE);
                strcpy(message, "Operazione TFTP non prevista\0");
                pos = senderror(errorCode, bufferError, fileName,message);
                newsd =  socket(AF_INET, SOCK_DGRAM, 0);

              ret = sendto(newsd, bufferError, pos, 0,(struct sockaddr*)&client_addr, sizeof(client_addr));

              if(ret < 0){
                perror("\n[ERRORE]: Invio dei dati non riuscito\n");
                exit(0);
              }

              close(newsd);
              continue;

          } while(1){
            
              addrlen = sizeof(client_addr);
              memset(pacchetto, 0, BUFFER_SIZE);
        
              ret = recvfrom(newsd, pacchetto, ACK_SIZE, 0, (struct sockaddr*)&client_addr, &addrlen);
       
          if(ret < 0){
              perror("Errore nella receive.\n");
              exit(0);
          }

              uint16_t opcode;
              memcpy(&opcode, pacchetto, 2);
        
              opcode = ntohs(opcode);

          if(opcode == 4){
              //printf("\n[DEBUG]: ACK ricevuto.\n");

             if (dim_rimasti > 0){
              
              unsigned int dim_pckt = (dim_rimasti > FILE_BUFFER_SIZE)?FILE_BUFFER_SIZE:dim_rimasti;
              blocksucc++;
          
              dim_rimasti -= dim_pckt;
             // printf("\r[DEBUG]: Invio del blocco [%d]\n",blocksucc);  
               // Lettura ed invio di un blocco
              uint16_t block_num = htons(blocksucc);

              if(!strcmp(mode, "netascii\0")){

              pos = sendtext( block_num, pacchetto, buffer_file, fp, dim_pckt);
              ret = sendto(newsd, pacchetto, pos, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));

              } else{
                 
               pos = sendbin( block_num, pacchetto, buffer_bin, fp, dim_pckt);
               ret = sendto(newsd, pacchetto, pos, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
                 }
                
               }else {

               memset(ip_client, 0, IP_SIZE);
               inet_ntop(AF_INET, &client_addr, ip_client, IP_SIZE);
               printf("\nL'intero file è stato trasferito con successo al client %s\n", ip_client);
              // printf("DISTRUGGI PROCESSO \n");
               valid = 0;
               exit(1);
               break;
                } 
              }
            }
           }if(pid > 0){
                   
                   continue;
           }
         }
            
       }


              






