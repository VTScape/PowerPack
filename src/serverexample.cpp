#include "functionapi.h"

void start(){
   std::cout << "me start\n"; 
}

void end(){
std::cout << "me end\n";
}

void tag(){
    std::cout << "me tag\n";
}


int main(int argc, char** argv){
    std::string configFile(argv[1]);
    socketServer server = initializeMeterServer(readServerConfig(configFile), start, end, tag);
    server.listenForClient();
    return 0;
} 

