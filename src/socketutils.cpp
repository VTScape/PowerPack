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

  if (listen(sock, 1) == -1) {
    printError("Server failed to listen on socket");
  }

  clientLength = sizeof(address);
  if ((readSocket = accept(sock, (sockaddr *)&address, &clientLength)) == -1) {
    printError("Error, the accept failed with errno: ");
  }

  while (handleClientConnection(readSocket) != -1)
    ;
}

int socketServer::handleClientConnection(int readSocket) {
  int msgTypeBuffer[1];
  readData(readSocket, msgTypeBuffer, sizeof(int));

  if (msgTypeBuffer[0] == SESSION_START) {
    handleSessionStart();
  } else if (msgTypeBuffer[0] == SESSION_END) {
    handleSessionEnd();
    return -1;
  } else if (msgTypeBuffer[0] > 0) {
    handleTag(readSocket, msgTypeBuffer[0]);
  } else {
    printError("Server received unknown msg code");
  }

  return 0;
}

void socketServer::handleSessionStart() {
  timestamps.emplace_back("sessionStart", nanos());
  std::cout << "session start\n";
}

void socketServer::handleSessionEnd() {
  timestamps.emplace_back("sessionEnd", nanos());
  std::cout << "session end\n";
  for (size_t index = 0; index < timestamps.size(); index++) {
    std::cout << "Tag: " << timestamps[index].first << " happened "
              << timestamps[index].second - timestamps[0].second
              << " nanoseconds after the program start.\n";
  }
}

void socketServer::handleTag(int socketFD, int size) {
  timestamps.emplace_back("", nanos());
  std::cout << nanos();
  std::cout << "session tag\n";
  char *msgBuffer = new char[size];
  readData(socketFD, msgBuffer, (size_t)size);
  std::cout << "tag is: '" << msgBuffer << "'\n";
  timestamps.back().first = msgBuffer;
  delete[] msgBuffer;
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

void socketClient::sendSessionStart() {
  int startBuf[1] = {SESSION_START};
  write(startBuf, sizeof(int));
}

void socketClient::sendSessionEnd() {
  int endBuf[1] = {SESSION_END};
  write(endBuf, sizeof(int));
}

void socketClient::sendTag(std::string tagName) {
  int tagBuf[1] = {(int)tagName.size()};
  write(tagBuf, sizeof(int));
  write((void *)tagName.c_str(), tagName.size());
}