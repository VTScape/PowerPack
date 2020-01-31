#include "base.h"

class derivedClass : public baseClass{
    public:
        derivedClass(void);
        virtual void mePrint();

    private:
        std::string msg;
};