#include "socketutils.h"
/*
* API For users who want to generate a power profile for their functions
* Client and server are two processes
* Client sends timestamped tags to server
* Server handles data collection via ni-meter
*/

void initializeMeterServer();
void closeMeterServer();


void initializeFunctionClient();
void closeFunctionClient();
void issueTag(std::string tagName);