/*
 *   * tcpserver.c - A simple TCP latency server
 *   * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFSIZE (1024*1024*1)
#define ITERATION 100000
char* sendbuf = NULL;
char* recvbuf = NULL;

int main(int argc, char** argv) {
    int serverfd; //  server socket
    int clientfd; // client socket
    int portnum; /*  port to listen on */
    int clientlen; /*  byte size of client's address */
    struct sockaddr_in serveraddr; /*  server's addr */
    struct sockaddr_in clientaddr; /*  client addr */
    //struct hostent *hostp; /*  client host info */
    char* hostaddrp; /*  dotted decimal host addr string */
    //int optval; /*  flag value for setsockopt */
    long n, tot = 0; /*  message byte size */

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }

    portnum = atoi(argv[1]);

    serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverfd < 0) {
        printf("Create parent socket error.\n");
        return -1;
    }

    memset((char*) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portnum);

    if (bind(serverfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("Bind server address error.\n");
        return -1;
    }
    //max num of connect is 5
    if (listen(serverfd, 5) < 0) {
        printf("Listen port error.\n");
        return -1;
    }

    clientlen = sizeof(clientaddr);
    clientfd = accept(serverfd, (struct sockaddr*)&clientaddr, (socklen_t*)&clientlen);

    if (clientfd < 0) {
        printf("Accept a wrong connection.\n");
        return -1;
    }

    hostaddrp = inet_ntoa(clientaddr.sin_addr);

    sendbuf = (char*)malloc(MAX_BUFSIZE);
    recvbuf = (char*)malloc(MAX_BUFSIZE);

    if ((sendbuf == NULL) || (recvbuf == NULL)) {
        printf("malloc buffer error.\n");
        return -1;
    }

    //Start testing
    int size = 0;
    int count = 0;

    for(count = 0; count < ITERATION; count++){
	n = read(clientfd, recvbuf, 1);

        if (n < 0) {
        	printf("iteration[%d]: recv data error.\n", count);
        	return -1;
    	}
	if (recvbuf[0] != (char)(count % 128)) {
                printf("Error\n");
        }

	sendbuf[0] = (char)(count % 128);

	n = write(clientfd, sendbuf, 1);

        if (n < 0) {
               printf("iteration[%d]: send data error.\n", count);
               return -1;
        }
    }

    close(serverfd);

    if (sendbuf) {
        free(sendbuf);
    }

    if (recvbuf) {
        free(recvbuf);
    }

    return 0;
}
