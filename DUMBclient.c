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
		printf("%s\n", hello);
	}else{
		//connected = 0;
		
	}

	char commandBuffer[128];
	char receiveBuffer[128];
	do{
		printf("\n> ");
		fgets(commandBuffer,256,stdin);
		if(strncmp(commandBuffer, "quit", 4) == 0){
			send(client_socket,"GDBYE",5,0);
			connected = 0;
			break;
		}
		send(client_socket,commandBuffer,strlen(commandBuffer),0);
		//sleep(1);
		//recv(client_socket, hello, sizeof(hello), 0);
		//printf("%s\n", hello);
		
		if(connected ==0)
			break;
			
	}while(connected == 1);
	return 0;
}
