#include "derived.h"

derivedClass::derivedClass(){
    msg = "hi";
}

void derivedClass::mePrint(){
    std::cout << msg << std::endl;
}