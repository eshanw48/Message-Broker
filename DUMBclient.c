#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

int connected = 1;

int main(int argc, char* argv[]) {
	int pNum;
	char *IPbuffer;
	char* ip = argv[1];
	char* portNum = argv[2];
	pNum = atoi(portNum);
	struct hostent *host_entry;
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;

	int client_socket;
	client_socket = socket(AF_INET,SOCK_STREAM,0);

	host_entry = gethostbyname(ip);
	if(host_entry == NULL)
	{
		error("ERROR: no such host\n");
	}

	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char*) host_entry -> h_addr,(char *)&server_address.sin_addr.s_addr,host_entry->h_length);
	server_address.sin_port = htons(pNum);


 	int retries=0;

	while(connect(client_socket,(struct sockaddr *) &server_address,sizeof(server_address)) < 0)
	{
		printf("Unable to connect. Retrying...\n");
		sleep(1);
        	if(retries >= 2){
            		printf("ERROR: Could not connect to server. Shutting down...\n");
	    		connected = 0;
           		return -1;
        	}
        	retries++;
	}
	send(client_socket,"HELLO",5,0);
	sleep(1);
	char hello[128];
	recv(client_socket, hello, sizeof(hello), 0);
	if(strncmp(hello, "HELLO DUMBv0 ready!", 19) ==0){
		printf("Successfully connected to server!\n");
	}else{
		//connected = 0;

	}

	char prevCommand[128];
	char commandBuffer[128];
	char receiveBuffer[128];
	char inputBuffer[128];

	do{
		bzero(commandBuffer, 128);
		bzero(receiveBuffer, 128);
		bzero(inputBuffer, 128);
		printf("\n> ");
		fgets(commandBuffer,128,stdin);
		strtok(commandBuffer,"\n");
		if(strncmp(commandBuffer, "quit", 4) == 0){
			send(client_socket,"GDBYE",5,0);
			connected = 0;
			break;
		}
		else if(strncmp(commandBuffer, "create", 6) == 0){
			printf("Okay, create message box with what name?\n> ");
			fgets(inputBuffer,128,stdin);
			strtok(inputBuffer,"\n");
			char out[] = "CREAT ";
			strcat(out, inputBuffer);
			send(client_socket,out,128,0);
			//printf("%s\n", out);
		}else if(strncmp(commandBuffer, "open", 4) == 0){
			printf("Okay, open which message box?\n> ");
			fgets(inputBuffer,128,stdin);
			strtok(inputBuffer,"\n");
			char out[] = "OPNBX ";
			strcat(out, inputBuffer);
			send(client_socket,out,128,0);
		}else if(strncmp(commandBuffer, "next", 4) == 0){
			send(client_socket, "NXTMG", 5,0);
			recv(client_socket, receiveBuffer, sizeof(receiveBuffer)-1, 0);
			if(strncmp(receiveBuffer, "ER:EMPTY", 8)==0){
				printf("Error: No more messages left in message box.\n");
			}else if(strncmp(receiveBuffer, "ER:NOOPN", 8)==0){
				printf("Error: No message box currently open.\n");
			}else if(strncmp(receiveBuffer, "ER:WHAT?", 8)==0){
				printf("Error: Command not sent correctly!\n");
			}else{//OK!12!Oh hai, Mark!
				
				char * length;
				length = strtok(receiveBuffer, "!");
				int len = strlen(length);
				int msgLength = atoi(length);
				int offset = msgLength + 4 ;
				char message[msgLength];
				strncpy(message, receiveBuffer + offset, msgLength);
				printf("Message: %s", message);
			}
		}
		else{
			send(client_socket,commandBuffer,sizeof(commandBuffer),0);
		}

		if(connected ==0)
			break;


		strcpy(commandBuffer, prevCommand);
	}while(connected == 1);
	return 0;
}
