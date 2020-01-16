#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <clocale>
#include <cstring>
#include <iostream>
#include <string>


// CONTROL MESSAGE MACROS

#define SESSION_START -2
#define SESSION_END -1

class socketServer {
 public:
  socketServer(int portNumber);

  ~socketServer();

  void listenForClient();

 private:

  // This is the file descriptor of the socket.
  int sock;

  sockaddr_in address;

  // This is a wrapper for socket read that stores the data in the given buffer.
  void readData(int socketFD, void* buf, size_t size);

  // This is a wrapper for socket write that stores
  void write(int socketFD, void* buf, size_t size);

  int handleClientConnection(int readSocket);

  void handleSessionStart();
  void handleSessionEnd();
  void handleTag(int socketFD);
};

class socketClient {
 public:
  socketClient(int portNumber, std::string serverIP);

  ~socketClient();

  void sendSessionStart();
  void sendSessionEnd();
  void sendTag(std::string tagName);

 private:
  // This is the file descriptor of the socket.

  void readData(void* buf, size_t size);

  void write(void* buf, size_t size);

  int sock;
  sockaddr_in serverAddress;
};

void printError(std::string errorMsg);
