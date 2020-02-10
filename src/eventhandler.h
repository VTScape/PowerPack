#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <stdio.h>
#include <iostream>
#include <string>

class eventHandler {
 public:
  eventHandler();
  virtual void startHandler() = 0;
  virtual void tagHandler() = 0;
  virtual void endHandler() = 0;
  virtual ~eventHandler();
};

#endif