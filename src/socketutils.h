#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>

class socketServer {
 public:
  socketServer(int portNumber);

  ~socketServer();

  void readData(void* buf, size_t size);

  void write(void* buf, size_t size);

  void listenForClient();

 private:
  // This is the file descriptor of the socket.
  int sock;
  sockaddr_in address;
};

class socketClient {
 public:
  socketClient(int portNumber, std::string serverIP);

  ~socketClient();

  void readData(void* buf, size_t size);

  void write(void* buf, size_t size);

 private:
  // This is the file descriptor of the socket.
  int sock;
  sockaddr_in serverAddress;
};