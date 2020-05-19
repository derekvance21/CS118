#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // for inet_ntoa
#include <string.h>
#include <unistd.h> // for close
#include <fcntl.h>
#include <sys/stat.h> // for file size

#define CHUNK_SIZE 4096
#define LISTENQ 20 // maximum number of client connections

#define SERV_PORT 9000


int main ()
{
	//creation of the socket
	int listenfd = socket (AF_INET, SOCK_STREAM, 0);

	//preparation of the socket address 
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY allows client to connect to ANY one of the host's IP address.
	servaddr.sin_port = htons(SERV_PORT);
	memset(servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero)); // it's 8 bytes of zeros I think

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr))==-1) {
		perror("bind");
		exit(1);
	}

	if (listen(listenfd, LISTENQ)==-1) {
		perror("listen");
		exit(1);
	}

	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int connfd;
	printf("%s\n","Server running...waiting for connections.");
	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
		printf("%s\n","Accepted client connection...");
		char object_buf[CHUNK_SIZE];
		char request_buf[CHUNK_SIZE];
		char response_buf[CHUNK_SIZE];
					
		int request_bytes = recv(connfd, request_buf, CHUNK_SIZE, 0);
		if (request_bytes < 0) {
			perror("Read error");
			exit(1);
		}
		printf("HTTP Request received from client:\n\n");
		puts(request_buf);

		int file_size = 0;
		int fd;
		if (strncmp(request_buf, "GET /", 5) != 0) {
			sprintf(response_buf, "HTTP/1.1 400 Bad Request\r\n");
		}
		else {
			char file_request[256]; // full name of file
			char file_format[64]; // file extension
			int i;
			int dotloc;
			int dot_found = 0;
			for (i = 5 /* length of "GET /" */; request_buf[i] != ' ' && i < request_bytes; i++) {
				if (dot_found)
					file_format[i - dotloc - 1] = request_buf[i];
				file_request[i - 5] = request_buf[i];
				if (request_buf[i] == '.') {
					dotloc = i;
					dot_found = 1;	
				}
			}
			file_request[i - 5] = '\0'; file_format[i - dotloc - 1] = '\0';
			fd = open(file_request, O_RDONLY);
			if (fd < 0) {
				sprintf(response_buf, "HTTP/1.1 404 Not Found\r\n");
			}
			else {
				if (!dot_found) strcpy(file_format, "application/octet-stream");
				else if (strcmp(file_format, "png") == 0) strcpy(file_format, "image/png");
				else if (strcmp(file_format, "jpg") == 0) strcpy(file_format, "image/jpeg");
				else if (strcmp(file_format, "txt") == 0) strcpy(file_format, "text/plain");
				else if (strcmp(file_format, "html") == 0) strcpy(file_format, "text/html");

				struct stat st;
				stat(file_request, &st);
				file_size = st.st_size;
				printf("file_size: %d\n", file_size);
				
				sprintf(response_buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", file_size, file_format);
			}
		}
		send(connfd, response_buf, strlen(response_buf), 0);
		int i;
		int chunks = file_size / 4096;
		for (i = 0; i < chunks; i++) {
            int chunk_size = read(fd, object_buf, 4096); // should always be 4096 anyways but alas
			send(connfd, object_buf, chunk_size, 0);
		}
        read(fd, object_buf, file_size % 4096);
		send(connfd, object_buf, file_size % 4096, 0);
		// memcpy(&response_buf[header_size], object_buf, obj_bytes);
		// int response_size = strlen(response_buf);
		// printf("bytes in response_buf: %d\n", response_size);
		// send(connfd, response_buf, header_size + obj_bytes, 0);

		memset(request_buf, '\0', CHUNK_SIZE);
		memset(object_buf, '\0', CHUNK_SIZE);
				
		printf("Close connection fd...\n\n");
		close(connfd);
	}
	//close listening socket
	printf("Close listening socket...\n");
	close (listenfd); 
	return 0;
}
