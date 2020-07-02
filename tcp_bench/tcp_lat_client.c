/*
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <arpa/inet.h>

/* max 1MB buffer to test */
#define MAX_BUFSIZE (1024*1024*8)
#define ITERATION 100000
char* sendbuf = NULL;
char* recvbuf = NULL;

int main(int argc, char** argv) {
    int sockfd, portnum;
    long n, tot = 0;
    struct sockaddr_in serveraddr;
    struct hostent* server;
    char* hostname;

    struct timeval start, sub;
    struct timeval end;

    /* check command line arguments */
    if (argc != 3) {
        fprintf(stderr, "usage: %s <hostname/ip> <port>\n", argv[0]);
        exit(0);
    }

    hostname = argv[1];
    portnum = atoi(argv[2]);
    sendbuf = (char*)malloc(MAX_BUFSIZE);
    recvbuf = (char*)malloc(MAX_BUFSIZE);

    if ((sendbuf == NULL) || (recvbuf == NULL)) {
        printf("Malloc buffer failed.\n");
        return -1;
    }

    /* socket: create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0) {
        printf("ERROR opening socket");
        return -1;
    }

    server = gethostbyname(hostname);

    if (server == NULL) {
        printf("ERROR, no such host as %s\n", hostname);
        return -1;
    }

    memset((char*) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portnum);
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("Connect to server error.\n");
        return -1;
    }

    int size = 0;
    int count = 0;
    printf("Unidirection tcp latency test\n #Size(B)  #Average Latency(us)\n");

    gettimeofday(&start, NULL);
    for(count = 0; count < ITERATION; count++){
	sendbuf[0] = (char)(count % 128);
	n = write(sockfd, sendbuf, 1);

        if (n < 0) {
	        printf("iteration[%d]:write socket error.\n", count);
                return -1;
        }

	n = read(sockfd, recvbuf, 1);

        if (n < 0) {
                printf("iteration[%d]:read socket error.\n", count);
                return -1;
        }

	if (recvbuf[0] != (char)(count % 128)) {
                printf("Error");
        }

	
    }

    gettimeofday(&end, NULL);
    timersub(&end, &start, &sub);
    printf("%-10d \t %-10lf \n", size,
               ((double)(sub.tv_sec * 1000000 + sub.tv_usec)) / ITERATION);

    close(sockfd);

    if (!sendbuf) {
        free(sendbuf);
    }

    if (!recvbuf) {
        free(recvbuf);
    }

    return 0;
}
