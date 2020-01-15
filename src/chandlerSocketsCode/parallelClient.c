/////////////////////////////////////////////////////////////
// Filename: parallelClient.c
// Author: Chandler Jearls
// Date: 12/15/2018
// Description: The parallelClient is not actually parallel, it is just a client for the parallelServer
// code. It simply accepts a hostname, then a port number, then a
// file to stream from and a file to stream to
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include "fileLib.c"


void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct timespec startTime, endTime;
    
    if (argc < 5) {
        // throws an error if there is an incorrect number of arguments given
       fprintf(stderr,"Can you please just conform to the usage: %s hostname port fileToStreamFrom fileToStreamTo\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]); // stores the port number of the server
    // Socket usage: socket(int domain, int type, int protocol)
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // creates a socket
    // AF_INET uses IPv4 to communicate (AF_INET6 uses IPv6 to communicate)
    // SOCK_STREAM configures the connection to use reliable
    // 0 uses the default protocol for the domain and type selected
    if (sockfd < 0) 
        error("ERROR opening socket"); // prints to sderr 
    server = gethostbyname(argv[1]); // stores the host name
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n"); // if there is a problem finding the host name, make an error
        exit(0);
    }

    // finishes setting up the communications
    bzero((char *) &serv_addr, sizeof(serv_addr)); // zeroes the server address holder
    serv_addr.sin_family = AF_INET; // tells the server address struct to use IPv4
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length); // copies the server's h_addr to the serv_addr.sin_addr.s_addr
    serv_addr.sin_port = htons(portno); // converts the unsigned short integer hostshort from host byte order to network byte order.
    clock_gettime(CLOCK_REALTIME, &startTime);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) // attempts to connect sockfd to 
        error("ERROR connecting"); // displays an error if it happens
    
    printf("Starting streaming from file\n");
    streamFromFile(argv[3], sockfd);
    printf("Done streaming from file\n");
    streamToFile(argv[4], sockfd);
    printf("Done streaming to file\n");
    
    
    clock_gettime(CLOCK_REALTIME, &endTime);
    // prints the amount of time from sending the request to receiving the sorted array
    printf("%f\n", endTime.tv_sec-startTime.tv_sec + ((endTime.tv_nsec - startTime.tv_nsec)/1000000000.0));
    return 0;
}