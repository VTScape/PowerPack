#include <thread>
#include "socketutils.h"

void setServerToListen(int serverPort) {
  socketServer server(serverPort);
  server.listenForClient();
}

int main() {
  int port = 8080;
  std::thread serverThread(setServerToListen, port);

  // This sleeps to be sure the server has enough time to start listening.
  sleep(1);

  // This starts the client and connects it to the localhost.
  socketClient client(port, "127.0.0.1");

  // This tests sending some data to the server.
  client.sendSessionStart();
  client.sendTag("tag1");
  client.sendTag("tag2");
  client.sendTag("tag3");
  client.sendSessionEnd();

  // This makes sure that the separate thread is correctly destroyed at the end
  // of the test.
  serverThread.join();
}