#include <stdio.h>
#include <iostream>
#include <string>

class baseClass {
 public:
  virtual void mePrint() = 0;
};

class derivedClass : public baseClass{
    public:
        derivedClass(void);
        virtual void mePrint();
        ~derivedClass(void);
    private:
        std::string msg;
};

derivedClass::derivedClass(){
    msg = "hi";
}

void derivedClass::mePrint(){
    std::cout << msg << std::endl;
}

derivedClass::~derivedClass(){
    
}

int main (){
    derivedClass derived;
    baseClass *baseP = &derived;
    baseP->mePrint(); 
}