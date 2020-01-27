#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <stdio.h>
#include <iostream>
#include <string>

class eventHandler {
 public:
  virtual void startHandler();
  virtual void tagHandler();
  virtual void endHandler();
};
#endif