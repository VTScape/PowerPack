#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <clocale>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "timeutils.h"
#include "eventhandler.h"

// CONTROL MESSAGE MACROS

#define SESSION_START 0
#define SESSION_END 1
#define SESSION_TAG 2

class socketServer {
 public:
  socketServer(int portNumber, eventHandler *handler);

  ~socketServer();

  // This blocks until it receives a connection from a client then continues to
  // receive data from the client.
  void listenForClient();

 private:
  // This is the file descriptor of the socket.
  int sock;
  eventHandler *handler;

  // This stores information about the server that is being connected to.
  sockaddr_in address;

  // This stores a list of timestamps and their identifying strings.
  std::vector<std::pair<std::string, u_int64_t>> timestamps;

  // This is a wrapper for socket read that stores the data in the given buffer.
  void readData(int socketFD, void* buf, size_t size);

  // This is a wrapper for socket write that sends data from the given buffer.
  void write(int socketFD, void* buf, size_t size);

  // This function interprets the data received in listenForClient() and calls
  // the correct function to handle the communication.`
  int handleClientConnection(int readSocket);

  // This does the things needed at a session start such as starting
  // the meter and taking the first timestamp.
  void handleSessionStart();

  // This does the things needed at the end of a session such as
  // stopping the meter and dumping the timestamps and meter readings to a file.
  void handleSessionEnd();

  // This marks the timestamp and string of a tag that has been
  // received.
  void handleTag(int socketFD);
};

class socketClient {
 public:
  socketClient(int portNumber, std::string serverIP);

  ~socketClient();

  // This lets the server know to start measuring.
  void sendSessionStart();

  // This lets the server know to stop measuring.
  void sendSessionEnd();

  // This tells the server to tag a specific time in the measurements while
  // something of note is happening.
  void sendTag(std::string tagName);

 private:
  // This is the file descriptor of the socket.
  int sock;

  // This stores information about the server that is being connected to.
  sockaddr_in serverAddress;

  // This is a wrapper for socket read that stores the data in the given buffer.
  void readData(void* buf, size_t size);

  // This is a wrapper for socket write that sends data from the given buffer.
  void write(void* buf, size_t size);
};

// This function prints an error message, prints the last errorno, and exits the
// program.
void printError(std::string errorMsg);

#endif