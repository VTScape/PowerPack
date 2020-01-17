#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include "socketutils.h"
/*
 * API For users who want to generate a power profile for their functions
 * Client and server are two processes
 * Client sends timestamped tags to server
 * Server handles data collection via ni-meter
 */

socketServer initializeMeterServer(int portNumber);
void closeMeterServer();
int readServerConfig(std::string configFilePath);

socketClient initializeFunctionClient(std::pair<int, std::string> clientNetworkInfo);
void closeFunctionClient();
void issueTag(std::string tagName);
std::pair<int, std::string> readClientConfig(std::string configFilePath);

bool isSubString(std::string inputString, std::string subString);
std::string extractConfigValue(std::string inputString);