#include "socketutils.h"

socketServer::socketServer(int portNumber) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(portNumber);
  int opt = 1;

  if ((sock = socket(address.sin_family, SOCK_STREAM, 0)) == 0) {
    std::cerr << "Server failed to initialize socket\n";
    exit(EXIT_FAILURE);
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    std::cerr << "Server failed to set socket options\n";
    exit(EXIT_FAILURE);
  }

  if (bind(sock, (sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Server failed to bind to socket\n";
    exit(EXIT_FAILURE);
  }
}

socketServer::~socketServer() { close(sock); }

void socketServer::readData(void *buf, size_t size) {
  if (read(sock, buf, size) == -1) {
    std::cerr << "Server failed to completely read from the socket\n";
    exit(EXIT_FAILURE);
  }
}

void socketServer::write(void *buf, size_t size) {
  size_t bytesSent;
  if((bytesSent = send(sock, buf, size, 0) < size){
    std::cerr << "Server failed to completely send on socket. Server sent "
              << bytesSent << " bytes instead of " << size << " bytes.\n";
    exit(EXIT_FAILURE);
  }
}

void socketServer::listenForClient() {
  socklen_t clientLength;
  int readSocket;
  while (listen(sock, 1) != -1) {
    clientLength = sizeof(address);
    readSocket = accept(sock, (sockaddr *)&address, &clientLength);
    if (handleClientConnection == -1) {
      break;
    }
  }
}

socketClient::socketClient(int portNumber, std::string serverIP) {
  sock = 0;

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(portNumber);

  if (sock = socket(serverAddress.sin_family, SOCK_STREAM, 0) < 0) {
    std::cerr << "Client socket creation error\n";
    exit(EXIT_FAILURE);
  }

  if (inet_pton(serverAddress.sin_family, serverIP.c_str(),
                &serverAddress.sin_addr)) {
    std::cerr << "Invalid address or address not supported\n";
    exit(EXIT_FAILURE);
  }

  if (connect(sock, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Client connection failed\n";
    exit(EXIT_FAILURE);
  }
}

socketClient::~socketClient() { close(sock); }

void socketClient::readData(void *buf, size_t size) {
  if (read(sock, buf, size) == -1) {
    std::cerr << "Client failed to completely read from socket\n";
    exit(EXIT_FAILURE);
  }
}

void socketClient::write(void *buf, size_t size) {
  size_t bytesSent;
  if((bytesSent = send(sock, buf, size, 0) < size){
    std::cerr << "Client failed to completely send on socket. Client sent "
              << bytesSent << " bytes instead of " << size << " bytes.\n";
    exit(EXIT_FAILURE);
  }
}