#include <thread>
#include "socketutils.h"

void setServerToListen(int serverPort) {
  socketServer server(serverPort);
  server.listenForClient();
}

int main() {
  int port = 8080;
  std::thread serverThread(setServerToListen, port);
  sleep(1);
  socketClient client(port, "127.0.0.1");
  int buf[] = {17};

  client.write(buf, sizeof(int));


}