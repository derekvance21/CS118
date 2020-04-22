#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 9000 /*port*/

#define HTTPREQUEST

char httpRequest[MAXLINE];

int main(int argc, char **argv) {
    //basic check of the arguments
    //additional checks can be inserted
    if (argc !=3) {
        perror("Usage: client <IP address of the server> <object requested>"); 
        exit(1);
    }

    //Create a socket for the client
    //If sockfd<0 there was an error in the creation of the socket
    int sockfd;
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
    }

    struct sockaddr_in servaddr;
    //Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr= inet_addr(argv[1]);
    servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order

    //Connection of the client to the socket 
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
        perror("Problem in connecting to the server");
        exit(3);
    }
    //Client connected to server

    char sendline[MAXLINE], recvline[MAXLINE];

    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        send(sockfd, sendline, strlen(sendline), 0);

        if (recv(sockfd, recvline, MAXLINE,0) == 0){
        //error: server terminated prematurely
            perror("The server terminated prematurely"); 
            exit(4);
        }
        printf("%s", "String received from the server: ");
        fputs(recvline, stdout);

        if (strcmp(sendline,"q\n")==0) {
        printf("Close connection...\n");
        break;
        }

        memset(sendline, '\0', MAXLINE);
        memset(recvline, '\0', MAXLINE);
    }

    close(sockfd);
    return 0;
}