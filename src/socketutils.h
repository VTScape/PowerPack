#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

class socketServer {
 public:
  socketServer(int portNumber);

  ~socketServer();

  void read(double size, void* buf);

  void write(double size, void* buf);

 private:
  // This is the file descriptor of the socket.
  int fd;
  sockaddr_in address;
};

class socketClient {
 public:
  socketClient(int portNumber);

  ~socketClient();

  void read(double size, void* buf);

  void write(double size, void* buf);

 private:
  // This is the file descriptor of the socket.
  int sock;
  sockaddr_in serverAddress;
};