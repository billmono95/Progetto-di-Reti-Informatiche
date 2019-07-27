#include "tftp_client.h"
int main(int argc, char** argv){

	if(argc != 3){
		printf("\nDigita: ./tftp_client <IP_server> <porta_server>\n");
		return 0;
	}

	int sd, port;
	
	struct sockaddr_in server_addr;
    
	char cmd[CMD_SIZE];

	port = atoi(argv[2]);

	sd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    char transfermode[CMD_SIZE]; 
	strcpy(transfermode,"octet\0");

	initMessage(sd, argv[1], port, server_addr); 

	while(1){

         memset(&cmd, 0, CMD_SIZE);
         scanf("%s", cmd);    

         if(!strcmp(cmd, "!help\0")){
			help();
		 }else if(!strcmp(cmd, "!mode\0")){ 
			char newmode[BUFFER_SIZE];
			scanf("%s",newmode); 
			mode(newmode, transfermode);
		 }else if(!strcmp(cmd, "!get\0")){

		    char file_name[BUFFER_SIZE], local_name[BUFFER_SIZE];
		    memset(&file_name, 0, BUFFER_SIZE);
			memset(&local_name, 0, BUFFER_SIZE);
			scanf("%s %s",file_name,local_name);	
			get(sd, file_name, local_name, transfermode, server_addr);

		 }else if(!strcmp(cmd, "!quit\0")){
			quit(sd);
		 }else{
			printf("\nOperazione non prevista, digita !help per la lista dei comandi\n");	
		    }
		printf(">");
	}
 return 0;
}

void initMessage(int sd, const char* server, int port, struct sockaddr_in server_addr){
	printf("Stai comunicando con %s alla porta %d effettuata con successo\n", server,port);
	help();
	printf(">");
	}

void help(){
	printf("\nSono disponibili i seguenti comandi:\n");
	printf("!help --> mostra l'elenco dei comandi disponibili\n");
	printf("!mode {txt|bin} --> imposta il modo di trasferimento dei files (testo o binario)\n");
	printf("!get filename nome_locale --> richiede al server il nome del file <filename> e lo salva localmente con il nome <nome_locale>\n");
	printf("!quit --> termina il client\n");
}
void mode(char* newmode, char* currentmode){
	if(!strcmp(newmode, "txt")){	
		printf("Modo di trasferimento testuale configurato.\n");
		strcpy(currentmode,"netascii\0");
	} else if(!strcmp(newmode, "bin")){
		printf("Modo di trasferimento binario configurato.\n");
		strcpy(currentmode,"octet\0");
	} else if(!strcmp(newmode, " ")){
		printf("ERRORE: Modo di trasferimento non previsto, inserire {txt|bin}.\n");
	}else{
		printf("ERRORE: Modo di trasferimento non previsto, inserire {txt|bin}.\n");
	}
	return;	
}

void get(int sd, char* file_name, char* local_name, char* transfermode, struct sockaddr_in server_addr){
    printf("\nRichiesta file %s al server in corso.\n", file_name);
    FILE* fp;
	if(!strcmp(transfermode, "netascii\0")){
		fp = fopen(local_name, "w+");   //creo il file mio dove ci metterò la roba
	}else{
		fp = fopen(local_name, "wb+");
	}if(fp == NULL){
		printf("\nErrore nell'apertura del file.\n");
		return;
	}

    char buffer[BUFFER_SIZE];
    unsigned int addrlen = sizeof(server_addr);
	memset(&buffer, 0, BUFFER_SIZE); 
	uint16_t opcode;
    long long transferiti = 0; 
	// Messaggio  RRQ
    int lenght = RRQ( buffer, file_name, transfermode);
    
	int ret = sendto(sd, buffer, lenght, 0,(struct sockaddr*)&server_addr, sizeof(server_addr));

	if(ret < 0){
		perror("[ERRORE]: send errata.\n");
		exit(0);
	}
    char dati[BUFFER_SIZE];
	char ack[BUFFER_SIZE];

    while(1){			
		// Ricezione del blocco	
		char pacchetto[BUFFER_SIZE];
		memset(&pacchetto, 0, BUFFER_SIZE);	
	    int posizione = 0;
	    memset(&dati, 0, FILE_BUFFER_SIZE); 

        int bytericevuti = recvfrom(sd, pacchetto, PACKET_SIZE, 0, (struct sockaddr*)&server_addr, &addrlen);
		
		if(bytericevuti < 0){
			perror("[ERRORE]: errore durante la ricezione dei dati.");
			exit(0);
		}
        memcpy(&opcode, pacchetto, 2);
		opcode = ntohs(opcode); 
		posizione += 2;
         // Errore
		if(opcode == 5){ 
			uint16_t error_number;

			memcpy(&error_number, pacchetto+posizione, 2);
			error_number = ntohs(error_number);
			posizione += 2;

			char error_msg[BUFFER_SIZE];
			memset(&error_msg, 0, BUFFER_SIZE);  
			strcpy(error_msg, pacchetto+posizione);

			printf("\n[ERRORE]: (%d) %s\n",error_number, error_msg);
			remove(local_name); 
			return;
		}
        if(transferiti == 0)
		printf("\nTrasferimento del file in corso.\n");
		// Lettura del blocco
		uint16_t block;
		memcpy(&block, pacchetto + posizione, 2);
		block = ntohs(block);
		posizione += 2;
		// printf("\r[DEBUG]: Ricevuto blocco [%d]\n",block);  
		if(!strcmp(transfermode, "netascii\0")){
			
			strcpy(dati, pacchetto+posizione);//ricopio in dati
			fprintf(fp, "%s", dati);//scrivo sul file
			}else{
			
			memcpy(dati, pacchetto+posizione, FILE_BUFFER_SIZE);//ricopio in dati la struttura
			fwrite(&dati, bytericevuti-4, 1 ,fp); // -4 per eliminare opcode e block_number dai byte ricevuti cioè 516 del packetsize
		}
        // Invio dell'ACK
        block = htons(block);	
        int lenght = acksend(ack,block);

		//printf("\r[DEBUG]: Inviato ACK blocco [%d]\n",htons(block)); 

		ret = sendto(sd, ack,lenght, 0,(struct sockaddr*)&server_addr, sizeof(server_addr));

		transferiti++;
        if(bytericevuti< PACKET_SIZE){
			printf("\nTrasferimento del file completato (%llu/%llu blocchi).\n", transferiti, transferiti);
			printf("\nSalvataggio %s completato.\n", file_name);
			fclose(fp);
			break;
		}
   }
}
void quit(int sd){
	close(sd);
	printf("\nDisconnessione effettuata.\n");
	exit(0);
}
