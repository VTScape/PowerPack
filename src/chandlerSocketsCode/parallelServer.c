/////////////////////////////////////////////////////////////
// Filename: parallelServer.c
// Author: Chandler Jearls
// Date: 12/16/2018
// Description: The parallelServer takes connections from clients, first receiving an integer
// telling how long the list of integers to sort will be, then receiving the list of integers
// to be sorted, then receiving an integer telling it how many sorts to perform before exiting
// 

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "sort.c"
#include "fileLib.c"

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void *handleClient(void *socketNumber){
    int newsockfd = (int) socketNumber;
    int listSize; // holds the number of integers in the list
    int n; // holds the outcomes of the reads and writes to the buffer
    n = read(newsockfd,&listSize,sizeof(int)); // read the number of integers in the list
    if (n < 0) error("ERROR reading buffer length from socket"); // throw an error
    int *buffer = malloc(sizeof(int)*listSize); // allocate memory for the list to be sorted
    n = read(newsockfd, buffer, sizeof(int)*listSize); // read in the list to be sorted into the buffer
    if (n < 0) error("ERROR reading buffer from socket"); // throw an error

    // sort the file
    struct inStruct sortStruct;
    sortStruct.arrayStart = buffer;
    sortStruct.arrayLength = listSize;
    sortStruct.layers = 3;
    sortStruct.scratchArray = NULL;
    pSort((void*) &sortStruct);
    // then return the sorted buffer
    n = write(newsockfd, buffer, sizeof(int)*listSize);
    if (n < 0) error("ERROR writing to buffer with socket"); // throw an error
    free(buffer);
    return NULL;
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n; // integer used to hold errors
    int listSize; // holds the length of the list to be sorted
    if (argc < 3) {
        // makes sure there are enough arguments provided
        fprintf(stderr,"ERROR, provide a port and a number of connections to accept\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // attempts to open a socket
    if (sockfd < 0) 
        error("ERROR opening socket"); // throws an error if the socket is unable to be opened

    bzero((char *) &serv_addr, sizeof(serv_addr)); // zeroes all the server address data
    portno = atoi(argv[1]); // converts the argument with the port number from a character string to an integer
    //setting the server's socket up
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding"); //if the bind doesn't work, throw an error
    

    int connectionCounter;
    int maxConnections = atoi(argv[2]);
    pthread_t *pThreadHolder = malloc(sizeof(pthread_t)*maxConnections);
    for(connectionCounter = 0; connectionCounter < maxConnections; connectionCounter++){
        listen(sockfd,5); // wait for a client to message the server
        clilen = sizeof(cli_addr); // find how many bits the client address type is
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //accept the connection 
        if (newsockfd < 0) 
            error("ERROR on accept"); // throw an error if the accept failed
        pthread_create(&pThreadHolder[connectionCounter], NULL, handleClient, (void*) newsockfd);
    }

    // join all the connections to be sure they all finish correctly
    for(connectionCounter = 0; connectionCounter < maxConnections; connectionCounter++){
        pthread_join(pThreadHolder[connectionCounter], NULL);
    }
    free(pThreadHolder);
}
    
        
  
