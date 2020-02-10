#include "functionapi.h"

socketServer initializeMeterServer(int portNumber, eventHandler *handler) {
  socketServer server(portNumber, handler);
  return server;
}

int readServerConfig(std::string configFilePath) {
  std::string portNumber;
  std::string tempString;
  std::ifstream configFile;

  configFile.open(configFilePath);

  while (!configFile.eof()) {
    getline(configFile, tempString);
    if (isSubString(tempString, "port")) {
      portNumber = extractConfigValue(tempString);
    } else {
      std::cerr << "Unrecognized value in server config file\n" << tempString;
      exit(EXIT_FAILURE);
    }
  }

  if (portNumber == "") {
    std::cerr << "server portNumber was not set\n";
    exit(EXIT_FAILURE);
  }

  return stoi(portNumber, nullptr, 10);
}

socketClient initializeFunctionClient(
    std::pair<int, std::string> clientNetworkInfo) {
  socketClient client(clientNetworkInfo.first, clientNetworkInfo.second);
  return client;
}

std::pair<int, std::string> readClientConfig(std::string configFilePath) {
  std::string portNumber = "";
  std::string IPAddress = "";
  std::string tempString;
  std::ifstream configFile;

  configFile.open(configFilePath);

  while (!configFile.eof()) {
    getline(configFile, tempString);
    std::cout << tempString << std::endl;
    if (isSubString(tempString, "port")) {
      portNumber = extractConfigValue(tempString);
    } else if (isSubString(tempString, "serveraddress")) {
      IPAddress = extractConfigValue(tempString);
    } else {
      std::cerr << "Unrecognized value in client config file\n";
      exit(EXIT_FAILURE);
    }
  }

  if (portNumber == "" || IPAddress == "") {
    std::cerr << "portNumber or IPAddress was not set\n";
    exit(EXIT_FAILURE);
  }

  return std::make_pair(stoi(portNumber, nullptr, 10), IPAddress);
}

std::string extractConfigValue(std::string inputString) {
  size_t delimPosition;
  std::cout << "extracting\n";
  if ((delimPosition = inputString.find("=")) == std::string::npos) {
    std::cerr << "Unable to find '=' in config file\n";
    exit(EXIT_FAILURE);
  }

  return inputString.substr(delimPosition + 1, std::string::npos);
}

bool isSubString(std::string inputString, std::string subString) {
  return inputString.find(subString) != std::string::npos;
}