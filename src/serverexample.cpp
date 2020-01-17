#include "functionapi.h"
#include "eventhandler.h"

int main(int argc, char** argv) {
  std::string configFile(argv[1]);
  eventHandler handler(argv[2]);
  socketServer server =
      initializeMeterServer(readServerConfig(configFile), handler);
  server.listenForClient();
  return 0;
}
