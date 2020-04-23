#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define RESPONSE_SIZE 1048576
#define REQUEST_SIZE 4096
#define SERV_PORT 9000 /*port*/


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

    char httpRequest[REQUEST_SIZE];
    char httpResponse[RESPONSE_SIZE];

    sprintf(httpRequest, "GET /%s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nUser-agent: Mozilla/5.0\r\n", argv[2]);

    send(sockfd, httpRequest, strlen(httpRequest), 0);
    int n;
    int bodyfound = 0;
    int fd = open("object", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    while ((n = recv(sockfd, httpResponse, RESPONSE_SIZE, 0)) > 0) {

    
    // if ((n = recv(sockfd, httpResponse, RESPONSE_SIZE, 0)) == 0) {
    //     //error: server terminated prematurely
    //     perror("The server terminated prematurely"); 
    //     exit(4);
    // }

        //printf("HTTP response received from the server:\n%s", httpResponse);
        //printf("\nSize of response: %d\n", n);
        if (bodyfound) {
            write(fd, httpResponse, n);
        }
        else {
            int i;
            for (i = 0; i < n - 3; i++) {
                if (httpResponse[i] == '\r' && httpResponse[i+1] == '\n' && httpResponse[i+2] == '\r' && httpResponse[i+3] == '\n') {
                    bodyfound = 1;
                    int body = i + 4;
                    printf("HTTP Response message: \n\n");
                    write(1, httpResponse, i);
                    printf("\n\n");
                    write(fd, &httpResponse[body], n - body);
                    break;
                }
            }
        }
    }
    printf("\nClosing connection...\n");
    close(sockfd);


    return 0;
}