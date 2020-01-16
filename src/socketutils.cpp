#include "socketutils.h"

void printError(std::string errorMsg) {
  std::cerr << errorMsg << std::strerror(errno) << "\n";
  exit(EXIT_FAILURE);
}

socketServer::socketServer(int portNumber) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(portNumber);
  int opt = 1;

  if ((sock = socket(address.sin_family, SOCK_STREAM, 0)) == 0) {
    printError("Server failed to initialize socket\n");
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    printError("Server failed to set socket options\n");
  }

  if (bind(sock, (sockaddr *)&address, sizeof(address)) < 0) {
    printError("Server failed to bind to socket\n");
  }
}

socketServer::~socketServer() { close(sock); }

void socketServer::readData(int socketFD, void *buf, size_t size) {
  if (read(socketFD, buf, size) == -1) {
    printError(
        "Server failed to completely read from the socket with errorno: ");
  }
}

void socketServer::write(int socketFD, void *buf, size_t size) {
  size_t bytesSent;
  if ((bytesSent = send(socketFD, buf, size, 0)) < size) {
    printError("Server failed to completely send on socket. Server sent");
  }
}

void socketServer::listenForClient() {
  socklen_t clientLength;
  int readSocket;
  while (listen(sock, 1) != -1) {
    clientLength = sizeof(address);
    readSocket = accept(sock, (sockaddr *)&address, &clientLength);
    if (readSocket == -1) {
      printError("Error, the accept failed with errno: ");
    }
    if (handleClientConnection(readSocket) == -1) {
      break;
    }
  }
}

int socketServer::handleClientConnection(int readSocket) {
  std::cout << "The readsocket file descriptor is: " << readSocket << "\n";

  int buffer[1];
  readData(readSocket, buffer, sizeof(int));

  std::cout << "The integer read was: " << *buffer << "\n";
  return 0;
}

socketClient::socketClient(int portNumber, std::string serverIP) {
  sock = 0;

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(portNumber);

  if ((sock = socket(serverAddress.sin_family, SOCK_STREAM, 0)) < 0) {
    printError("Client socket creation error\n");
  }

  if (inet_pton(serverAddress.sin_family, serverIP.c_str(),
                &serverAddress.sin_addr) == -1) {
    printError("Invalid address or address not supported\n");
  }

  if (connect(sock, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    printError("Client connection failed with errorno: ");
  }
}

socketClient::~socketClient() { close(sock); }

void socketClient::readData(void *buf, size_t size) {
  if (read(sock, buf, size) == -1) {
    printError("Client failed to completely read from socket\n");
  }
}

void socketClient::write(void *buf, size_t size) {
  size_t bytesSent;
  if ((bytesSent = send(sock, buf, size, 0)) < size) {
    printError("Client failed to completely send on socket. Client sent ");
  }
}