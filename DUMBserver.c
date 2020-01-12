#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

/* Structs needed. */

//For connection.
typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

//For the queue node.
typedef struct nodeQ {

    char * message ;
    struct nodeQ * next ;

} nodeQ;

//For the queue needed for the messages.
typedef struct Queue {

    struct nodeQ * begin ;
    struct nodeQ * end ;

} Queue ;

//For the link list needed to store the message boxes.
typedef struct linkList {

    char * name ;
    struct linkList * next ;
    Queue * q ;
    int isOpen ;

} linkList;

int portN; //The port number.

//Function to create the queue.
Queue * makeQueue() {

    Queue * output = (Queue*)malloc(sizeof(Queue)) ;
    output->begin = NULL ;
    output->end = NULL ;
    return output ;

}

//Function to create the link list.
linkList * createLL() {

    linkList * output = (linkList *)malloc(sizeof(linkList *)) ;
    output->next = NULL ;
    output->q = makeQueue() ;
    output->isOpen = 0 ;
    output->name = (char *)malloc(25 * sizeof(char)) ;
    return output ;

}

//Function to delete the link list.
void deletelinkList(linkList ** head,char * str){

linkList* temp = *head;
linkList* prev = *head;


if(temp!=NULL && strcmp(temp->name,str)==0){
  *head = temp->next;
  return;
}

while(temp!=NULL && strcmp(temp->name,str)!=0){
  prev = temp;
  temp = temp->next;
}

if(temp == NULL){
  return;
}

prev->next = temp->next;

}

//Function implimenting enqueue.
void Enqueue(Queue * q, char * msg) {

    struct nodeQ * node = (nodeQ *)malloc(sizeof(nodeQ)) ;
    node->message = (char *)malloc(strlen(msg) * sizeof(char)) ;
    strcpy(node->message, msg) ;
    node->next = NULL ;

    if (q->end == NULL) {

        q->begin = node ;
        q->end = node ;
        return ;

    }

    q->end->next = node ;
    q->end = node ;

}

//Function implimenting dequeue.
nodeQ * dequeue(Queue* q){

  if(q->begin==NULL){
    return NULL;
  }

  nodeQ* temp = q->begin;
  nodeQ* output = (nodeQ *)malloc(sizeof(nodeQ)) ;
  output->message = q->begin->message ;
  free(temp) ;

  q->begin = q->begin->next;

  if(q->begin==NULL){
    q->end=NULL;
  }

  return output;

}

linkList * messageBoxes = NULL ;

//Thread which is conncted to the server and goes through and impliments the commands.
void * process(void *arg) {

    /* Used to display the local time of the computer. */
    time_t t ; 
    struct tm *tmp ; 
    char MY_TIME[50]; 
    time( &t );
    tmp = localtime( &t );
    strftime(MY_TIME, sizeof(MY_TIME), "%H%M %d %B", tmp); 

    int cl = *((int*)arg) ;
    int r ;

    char buffer[5] ;
    r = read(cl, buffer, 5) ;
    printf("Got message: %s\n", buffer) ;
    char str[] = "HELLO DUMBv0 ready!" ;
    write(cl, str, sizeof(str)) ;
    printf("Sent out msg\n") ;

    char command[50] ;
    linkList * openBox ;

    while (1) { //The server should be on until the user hits control + c.
        printf("%s\n", MY_TIME ); //Print the system time.
        bzero(command, 50) ;
        r = read(cl, command, 50) ;

        if (strncmp(command, "CREAT ", 6) == 0) { //impliment create command.

            printf("Create\n") ;
            printf("command: %s\n", command) ;

            char * name ;
            name = strtok(command, " ") ;
            name = strtok(NULL, " ") ;

            printf("Name: %s\n", name) ;

            if (isalpha(name[0]) == 0 || strlen(name) < 5 || strlen(name) > 25) {

                write(cl, "ER:WHAT?", 9) ;

            
            } else {
    
            linkList * ptr ;

            if (messageBoxes == NULL) {

                messageBoxes = createLL() ;
                ptr = messageBoxes ;


            } else {

                linkList * temp = createLL() ;

                ptr = messageBoxes ;

                while (ptr->next != NULL && strcmp(ptr->name, name) != 0) {

                    ptr = ptr->next ;

                }

                if (strcmp(ptr->name, name) == 0) {

                    write(cl, "ER:EXIST", 9) ;
                    
                } else {

                    ptr->next = temp ;
                    ptr = ptr->next ;

                }

            }

            strcpy(ptr->name, name) ;


            ptr->q = makeQueue() ;

            write(cl, "OK!", 3) ;

            }


        } else if (strncmp(command, "OPNBX ", 6) == 0) { //impliment the open box command.

            if (messageBoxes != NULL) {

                if (openBox != NULL) {

                    printf("?\n") ;
                    write(cl, "ER:WHAT?", 9) ;

                } else {

                printf("messageBoxes: %s\n", messageBoxes->name) ;

                char * name ;
                name = strtok(command, " ") ;
                name = strtok(NULL, " ") ;
                printf("Looking for %s\n", name) ;

                if (isalpha(name[0]) == 0) {

                    write(cl, "ER:WHAT?", 9) ;

                } else {

                linkList * ptr = messageBoxes ;

                while (ptr != NULL && strcmp(ptr->name, name) != 0) {

                    printf("current: %s\n", ptr->name) ;
                    ptr = ptr->next ;

                }

                if (ptr != NULL) {

                    if (ptr->isOpen == 0) {

                        openBox = ptr ;
                        openBox->isOpen = 1 ;
                       
                        write(cl, "OK!", 3) ;

                    } else {

                     
                        write(cl, "ER:OPEND", 9) ;

                    }

                } else {

                    write(cl, "ER:NEXST", 9) ;

                }

                }

                }


            } else {

                write(cl, "ER:NEXST", 9) ;
                

            }

        } else if (strncmp(command, "NXTMG", 6) == 0) { //impliment the next message command.

            if (openBox != NULL) {

                nodeQ * msg = dequeue(openBox->q) ;

                if (msg != NULL) {
                    
                    char toClient[] = "OK!" ;
                    char n[5] ;
                    sprintf(n, "%d", strlen(msg->message)) ;
                    strcat(n, "!") ;
                    strcat(toClient, n) ;
                    strcat(toClient, msg->message) ;
                   
                    write(cl, toClient, strlen(toClient)) ;

                } else {

                    printf("error: no messages\n") ;
                    write(cl, "ER:EMPTY", 9) ;

                }

            } else {

                printf("error: no box is open\n") ;
                write(cl, "ER:NOOPN", 9) ;

            }

        } else if (strncmp(command, "PUTMG!", 6) == 0) { //impliment the put message command.

            if (openBox != NULL) {

            char * getSize ;
            getSize = strtok(command, "!") ;
            getSize = strtok(NULL, "!") ;

            if (getSize != NULL) {

            int sizeOfMsg = atoi(getSize) ;

            char * msg = (char *)malloc(sizeOfMsg * sizeof(char)) ;
            
            char * temp = strtok(NULL, "!") ;

            if (temp != NULL) {
                
                strcat(msg, temp) ;
                temp = strtok(NULL, "") ;
            
                if (temp != NULL) {

                    strcat(msg, "!") ;
                    strcat(msg, temp) ;

                }

            char m[sizeOfMsg] ;

            strcpy(m, msg) ;

                if (strlen(m) == sizeOfMsg) {
                    
                    Enqueue(openBox->q, m) ;
                    printf("open: %s\n", openBox->q->begin->message) ;
                    char ok[3] = "OK!" ;
                    strcat(ok, getSize) ;
                    write(cl, ok, sizeof(ok)) ;
                    printf("OK!%d ok: %s\n", sizeOfMsg, ok) ;

                } else {

                    

                }


            } else {

                printf("ER:WHAT?\n") ;

            }

            } else {

                printf("ER:WHAT?\n") ;

            }

            } else {

                write(cl, "ER:NOOPN", 9) ;
                

            }

            printf("current: %s\n", openBox->q->begin->message) ;

        } else if (strncmp(command, "CLSBX ", 6) == 0) { //impliment the close box command.

            if (messageBoxes != NULL) {

              printf("messageBoxes: %s\n", messageBoxes->name) ;

              char * name ;
              name = strtok(command, " ") ;
              name = strtok(NULL, " ") ;
              printf("Looking for %s\n", name) ;

              linkList * ptr = messageBoxes ;

              while (ptr != NULL && strcmp(ptr->name, name) != 0) {

                  printf("current: %s\n", ptr->name) ;
                  ptr = ptr->next ;

              }

              if (ptr != NULL) {

                  if (ptr->isOpen == 1 && openBox == ptr) {

                      openBox->isOpen = 0 ;
                      openBox = NULL ;
                      printf("OK!\n");
                      write(cl, "OK!", 3) ;

                  } else {

                      printf("error: is not open\n") ;
                      write(cl, "ER:NOOPN", 9) ;

                  }

              } else {

                  printf("error: couldn't find\n") ;
                  write(cl, "ER:NOOPN", 9) ;

              }

          } else {

              printf("error: not open\n") ;
              write(cl, "ER:NOOPN", 9) ;

          }

        } else if(strncmp(command, "DELBX ", 6) == 0){ //impliment the delete box command.

          if(messageBoxes!=NULL){

            char * name ;
            name = strtok(command, " ") ;
            name = strtok(NULL, " ") ;
            printf("Looking for %s\n", name) ;

            linkList * ptr = messageBoxes ;

            while (ptr != NULL && strcmp(ptr->name, name) != 0) {

                ptr = ptr->next ;

            }

          

            if (ptr == NULL) {

                write(cl, "ER:NEXST", 8) ;

            } else {

                if (ptr->q->begin != NULL) {
                
                    printf("not empty\n") ;
                    write(cl, "ER:NOTMT", 8) ;

                } else {

                    if (ptr->isOpen == 1) {

                    printf("error, cannot delete message box that is currently opened.\n");
                    write(cl, "ER:OPEND", 8) ;

                    } else {

                     deletelinkList(&messageBoxes,name);
                     printf("OKK!\n");
                     write(cl, "OK!", 3) ;

                    }

                }
            
            }

          }else{

            printf("error");
            write(cl, "ER:NEXST", 8) ;
          }

        } else if (strncmp(command, "GDBYE", 5) == 0) { // GOOD BYE 

            printf("quit\n") ;
            
            break ;

        } else {

            printf("pls\n") ;
            write(cl, "ER:WHAT?", 9) ;

        }

    }

   

    printf("quitting\n") ;

    close(cl) ;

}

int main(int argc, char ** argv) {
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

	printf("%s: ready and listening on port %d \n", argv[0], portN);

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
