#include <stdio.h>
#include <stdlib.h>

#define FILE_INTEGERS 100


void streamFromFile(char *fileName, int sockfd){
    /*
    Streams from a given file in batches of max size FILE_INTEGERS
    Does so by first sending the number of integers
    */
    FILE *fileToStream = fopen(fileName, "r");
    // we are storing integers, so make the buffer the size of one integer
    // plus one character, to hold the null character from the call to
    // fgets
    char *buffer = malloc(sizeof(int)*FILE_INTEGERS);
    //printf("Buffer is being created\n");
    int readCounter, n, keepSending;
    readCounter = FILE_INTEGERS;
    keepSending = 1;
    while(readCounter == FILE_INTEGERS ){
        readCounter = fread(buffer, sizeof(int), FILE_INTEGERS, fileToStream);
        ////printf("Sending the size of the current 'packet' %d\n", readCounter);
        n = write(sockfd, &readCounter, sizeof(int)); // writes how many integers to expect to the socket
        if (n < 0) {
            printf("The error is here\n");
            error("ERROR writing to socket\n");
            
        }
        //printf("Sending the actual 'packet'\n");
        n = write(sockfd, buffer, sizeof(int)*readCounter); // writes the integers to be sorted to the stream
        if (n < 0) {
            printf("No, the error is here\n");
            error("ERROR writing to socket\n");
            
        }


    }

    free(buffer);
    fclose(fileToStream);
}

void streamToFile(char *fileName, int sockfd){
    FILE *writeFile = fopen(fileName, "w");
    char *buffer = malloc(sizeof(int)*FILE_INTEGERS);

    int readCounter, n, keepReceiving, writeCounter;
    keepReceiving = 1;
    readCounter = FILE_INTEGERS;
    while(readCounter == FILE_INTEGERS){
        //printf("Doing another iteration, because the while loop has value %d\n", readCounter == FILE_INTEGERS*sizeof(int)/sizeof(char));
        //printf("Reading the size of the file to accept\n");
        n = read(sockfd, &readCounter, sizeof(int)); // read how many characters to receive
        //printf("The size of the file to accept was read to be %d\n", readCounter);
        if (n < 0) {
            //printf("The error might be here\n");
            error("ERROR reading from socket\n");
            
        }

        if(readCounter > 0){
            //printf("Reading the file from the socket\n");
            if(readCounter > FILE_INTEGERS){
                printf("The readCount was too high, stopping the program before a segfault, readcounter was %d\n", readCounter);
                error("ERROR, segfault imminent, readCounter too large\n");
            }
            n = read(sockfd, buffer, sizeof(int)*readCounter); // reads the integers to be written to the file
            if (n < 0) {
                printf("Or the error might actually be here %d\n", readCounter);
                error("ERROR reading from socket\n");
                
            }        
            
            //printf("Writing the file from the socket to the file\n");
            fwrite(buffer, sizeof(int), readCounter, writeFile);
        }
        

    }
    

    free(buffer);
    fclose(writeFile);
}