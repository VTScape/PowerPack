#include "functionapi.h"
#include <thread>

int clientThread(std::string configFile) {

  socketClient client = initializeFunctionClient(readClientConfig(configFile));

  client.sendSessionStart();

  client.sendTag("Hubba Hubba");
  std::cout << "Hello World!\n";
  client.sendTag("Done Printing");

  int a;
  client.sendTag("About to add some numbers");
  a = 2 + 2;
  sleep(1);
  client.sendTag("Done adding numbers");
  
  client.sendSessionEnd();
  return 0;
}


int main(int argc, char** argv) {
  std::string configFile(argv[1]);

  std::thread runHandler(clientThread, configFile);
  runHandler.join();
}

