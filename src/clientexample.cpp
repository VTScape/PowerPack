#include "functionapi.h"

int main(int argc, char** argv){   
    std::string configFile(argv[1]);
    socketClient client = initializeFunctionClient(readClientConfig(configFile));
    client.sendSessionStart();
    client.sendTag("About to print");
    std::cout << "Hello World!\n";
    client.sendTag("Done Printing");

    int a;
    client.sendTag("About to add some numbers");
    a = 2 + 2;
    client.sendTag("Done adding numbers");
    client.sendSessionEnd();
}