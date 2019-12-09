#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

typedef struct
{
	char *name;
	char **message;
	struct msgBox *next;
} msgBox;

msgBox *newMsgBox(char *name) {
	struct msgBox *root = (msgBox*)malloc(sizeof(msgBox));
  root->name = name;
  root->next = NULL;
  return root;
}

int portN;
msgBox *head = newMsgBox("head");

void * process(void * ptr)
{
	char * buffer;
	int len;
	connection_t * conn;
	long addr = 0;

	if (!ptr) pthread_exit(0);
	conn = (connection_t *)ptr;


	//HELLO
	printf("Port number is %d\n", portN);


	char *command = (char *)malloc(5*sizeof(char));
	char *command2 = (char*)malloc(25*sizeof(char));

	read(conn->sock,command,5);
	printf("command: %s\n", command);
	if(strcmp(command,"GDBYE")==0){
		shutdown(conn->sock);
		close(conn->sock);
		read(conn->sock, command, 5);
		free(conn);
		pthread_exit(0);
	}else if(strcmp(command,"CREAT")==0){
		read(conn->sock, command2, 25);
		int len = strlen(command2);
		if (len >= 5 && len <= 25) {
			if ((command2[0] >= 'a' && command2[0] <= 'z') || command2[0] >= 'A' && command2[0] <= 'Z') {
				msgBox temp = head;
				while (temp != NULL) {
					if (strcmp(temp->name, command2) == 0) {
						fprintf(stderr, "ER:EXIST\n");
						//pthread_exit(0);
					}
					temp = temp->next;
				}
			}
		}
	}
	/* close socket and clean up */
	/*close(conn->sock);
	free(conn);
	pthread_exit(0);*/

}

	int main(int argc, char **argv)
{
	int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;
	portN = atoi(argv[1]);

	/* check for command line arguments */
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/* listen on port */
	if (listen(sock, 5) < 0)
	{
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	printf("%s: ready and listening\n", argv[0]);

	while (1)
	{
		/* accept incoming connections */
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->sock = accept(sock, &connection->address, &connection->addr_len);
		if (connection->sock <= 0)
		{
			free(connection);
		}
		else
		{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}
	}

	return 0;
}
