#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* Structs needed. */
struct addrinfo h ;
struct addrinfo *iptr ;

int main(int argc, char ** argv) {

  if (argv[1] == NULL) { //no ip.

    printf("IP address is NULL.\n") ;
    return 0 ;

  }

  if (argv[2] == NULL) { //no port.

    printf("Port is NULL.\n") ;
    return 0 ;

  }

  printf("%s\n", argv[1]) ; //print ip.
  printf("%s\n", argv[2]) ;//print port.

  h.ai_family = AF_INET ;

  int addrinfo = getaddrinfo(argv[1], NULL, &h, &iptr) ;

  if (addrinfo) {

    printf("error?\n") ;
    
  }

  struct addrinfo *p ;
  char ip[256] ;

  for (p = iptr ; p != NULL ; p = p->ai_next) {

    getnameinfo(p->ai_addr, p->ai_addrlen, ip, sizeof(ip), NULL, 0, NI_NUMERICHOST) ;

  }

  printf("ip?: %s\n", ip) ;

  freeaddrinfo(iptr) ;

 /* Connecting to the server given the ip address and port number. */
  int sock = socket(AF_INET, SOCK_STREAM, 0) ;
  struct sockaddr_in serv_addr ;

  serv_addr.sin_family = AF_INET ;
  serv_addr.sin_addr.s_addr = inet_addr(ip) ;
  serv_addr.sin_port = htons(atoi(argv[2])) ; 
  if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {

    printf("Couldn't connect\n") ;
    return 0 ;

  } else {

    printf("Connected\n") ;

  }

  write(sock, "HELLO", 5) ;

  char buffer[50] ;

  read(sock, buffer, sizeof(buffer) - 1) ;

  printf("Got message: %s\n", buffer) ;

  if (strcmp(buffer, "HELLO DUMBv0 ready!") != 0) {

    printf("Error: got incorrect message\n") ;

  }

    char input[50] ;

    while (1) {

        bzero(input, 50) ;
        bzero(buffer, 50) ;
        printf("Waiting for Input: ") ;
        fgets(input, 50, stdin) ;
        strtok(input, "\n") ;

        if (strcmp(input, "create") == 0) {

          printf("Okay, what ddo you want to name the message box?\ncreate:> " );
          fgets(input, 50, stdin) ;
          strtok(input, "\n") ;

          char sendToServer[] = "CREAT " ; //create the message box.
          strcat(sendToServer, input) ;

          write(sock, sendToServer, 50) ;

          if (read(sock, buffer, sizeof(buffer) - 1) == -1) {

            printf("Error\n") ;

          } else {

            if(strncmp(buffer, "ER:EXIST", 8) == 0) { //if message box name is a duplicate.
              
              printf("Error. Message box '%s' already exists.\n", input) ;

            } else if (strncmp(buffer, "ER:WHAT?", 8) == 0) {

              printf("Error. Message is malformed.\n") ;

            } else {

              printf("Success! Message box '%s' was created.\n", input) ;

            }

          }

        } else if (strcmp(input, "open") == 0) { //open a message box.

          printf("Okay, open which message box?\nopen:> " );
          fgets(input, 50, stdin) ;
          strtok(input, "\n") ;

          char sendToServer[] = "OPNBX " ;
          strcat(sendToServer, input) ;
          printf("Sending %s\n", sendToServer) ;
          write(sock, sendToServer, 50) ;

          if (read(sock, buffer, sizeof(buffer) - 1) == -1) {

            printf("Error\n") ;

          } else {

            if (strncmp(buffer, "ER:NEXST", 8) == 0) { //message box does not exist.

              printf("Error. Message box '%s' does not exist.\n", input) ;

            } else if (strncmp(buffer, "ER:OPEND", 9) == 0) {

              printf("Error. Message box '%s' is already open.\n", input) ;

            } else if (strncmp(buffer, "ER:WHAT?", 9) == 0) {

              printf("Error. Command was unsuccessful, please try again.\n") ;

            } else {

              printf("Success! Message box '%s' is now open.\n", input) ;

            }

          }


        } else if (strcmp(input, "next") == 0) { //get next message box.
        
          write(sock, "NXTMG", 50);
          if (read(sock, buffer, sizeof(buffer) - 1) == -1) {

            printf("Error\n") ;

          } else {

            if (strncmp(buffer, "ER:EMPTY", 9) == 0) {

              printf("Error. There are no messages left in the message box.\n") ;              

            } else if (strncmp(buffer, "ER:NOOPN", 9) == 0) {  //no box open.

              printf("Error. You do not have a message box open.\n") ;

            } else {

            char * getSize ;
            getSize = strtok(buffer, "!") ;
            int ind = strlen(getSize) ;
            getSize = strtok(NULL, "!") ;
            ind = ind + strlen(getSize) + 2 ;

            int sizeOfMsg = atoi(getSize) ;

            char msg[sizeOfMsg] ;
            strncpy(msg, buffer + ind, sizeOfMsg) ;

            printf("Success! Message received: %s\n", msg) ;            
            

            }

          }


        } else if (strcmp(input, "put") == 0) { //put message in box.

          printf("Okay, what message do you want to put?\nput:> " );
          fgets(input, 50, stdin) ;
          strtok(input, "\n") ;

          char sendToServer[] = "PUTMG!" ;
          char n[2] ;
          sprintf(n, "%d", strlen(input)) ;
          strcat(sendToServer, n) ;
          strcat(sendToServer, "!") ;
          strcat(sendToServer, input) ;
          write(sock, sendToServer, 50) ;

          if (read(sock, buffer, sizeof(buffer) - 1) == -1) {

            printf("Error\n") ;

          } else {

            if (strncmp(buffer, "ER:NOOPN", 9) == 0) {

                printf("Error. You do not have a message box open.\n") ;

            } else if (strncmp(buffer, "ER:WHAT?", 9) == 0) {

              printf("Error. Command was unsuccessful, please try again.\n") ;

            } else {

              printf("Success! Message put into box: %s\n", input) ;

            }

          }


        } else if (strcmp(input, "close") == 0) { //close the message box.

          printf("Okay, close which message box?\nclose:> " );
          fgets(input, 50, stdin) ;
          strtok(input, "\n") ;

          char sendToServer[] = "CLSBX " ;
          strcat(sendToServer, input) ;
          printf("Sending %s\n", sendToServer) ;
          write(sock, sendToServer, 50) ;

          if (read(sock, buffer, sizeof(buffer) - 1) == -1) {

            printf("Error\n") ;

          } else {

            if (strncmp(buffer, "ER:NOOPN", 9) == 0) {

                printf("Error. You do not have that message box open.\n") ;

            } else if (strncmp(buffer, "ER:WHAT?", 9) == 0) {

              printf("Error. Command was unsuccessful, please try again.\n") ;

            } else {

              printf("Success! Box %s was closed.\n", input) ;

            }

          }

        } else if (strcmp(input, "delete") == 0)   { //delete the message box.

          printf("Okay, delete which message box?\ndelete:> " );
          fgets(input, 50, stdin) ;
          strtok(input, "\n") ;

          char sendToServer[] = "DELBX " ;
          strcat(sendToServer, input) ;
          printf("Sending %s\n", sendToServer) ;
          write(sock, sendToServer, 50) ;

          if (read(sock, buffer, sizeof(buffer) - 1) == -1) {

            printf("Error\n") ;

          } else {

            if (strncmp(buffer, "ER:NEXST", 8) == 0) {

                printf("Error. Message box '%s' does not exist.\n", input) ;

            } else if (strncmp(buffer, "ER:OPEND", 8) == 0) {

              printf("Error. Message box '%s' is currently open.\n", input) ;

            } else if (strncmp(buffer, "ER:NOTMT", 9) == 0 ) {

              printf("Error. Message box '%s' still has messages.\n", input) ;

            } else {

              printf("Success! Box %s was deleted.\n", input) ;

            }

          }

        } else if (strcmp(input, "quit") == 0) { //quit the program. GOOD BYE

          printf("quitting\n") ;
          write(sock, "GDBYE", 50) ;

          break ;

        } else {

          

        }

    }

  close(sock); //close the connection.

  return 0 ;

}
