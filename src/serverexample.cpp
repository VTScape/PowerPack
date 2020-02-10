#include <thread>
#include "functionapi.h"
#include "nidaqmxeventhandler.h"

int handlerThread(eventHandler* handler) {
  //sends event to thread

  handler->startHandler();
  handler->tagHandler();
  handler->tagHandler();
  handler->tagHandler();
  handler->tagHandler();
  handler->endHandler();

  return 0;
}

int main(int argc, char** argv) {
  std::string configFile(argv[1]);
  NIDAQmxEventHandler niHandler(argv[2]);
  eventHandler* handler;
  handler = &niHandler;

  socketServer server =
      initializeMeterServer(readServerConfig(configFile), handler);
  server.listenForClient();

  //handlers are handled in a thread because they aren't synchronous.
  //If they weren't in a thread, the main function would end, causing a
  //'virtual method teminated error.
  //std::thread runHandler(handlerThread, handler);
  //runHandler.join();

  return 0;
}
