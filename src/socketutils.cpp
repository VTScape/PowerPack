#include "socketutils.h"

socketServer::socketServer(int portNumber) {
  fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd == 0) {
    std::cerr << "Socket failed to initiallize\n";
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    std::cerr << "Failure to set socket options\n";
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(portNumber);

  if (bind(fd, (sockaddr*)&address, sizeof(address)) < 0) {
    std::cerr << "Socket failed to bind\n";
    exit(EXIT_FAILURE);
  }
}

socketServer::~socketServer() {
  close(fd);
}

void socketServer::read(double size, void* buf) {}

void socketServer::write(double size, void* buf) {}
