SHELL = /bin/sh

LIBS = nidaqmx

OS := $(shell uname)

ifeq ($(OS),Darwin)
LIBFLAGS = -framework $(LIBS)
else
LIBFLAGS = -l$(LIBS)
LDFLAGS += -L/usr/lib/x86_64-linux-gnu
endif

TARGET = helloworld

OBJS = $(TARGET).o

DEPS = nimeasure.o

LIBFLAGS += -lm

CDEBUG = -ggdb3
LDFLAGS += -g

CFLAGS += $(CDEBUG)

ifneq ($(filter $(OS), Linux Darwin),)
CFLAGS += -D_POSIX_C_SOURCE=200809L
endif

ifeq ($(OS),Darwin)
# Using #include <NIDAQmx.h> requires NIDAQmx.h to be in the include path.
# Using #include <nidaqmx/NIDAQmx.h> takes advantage of the fact that nidaqmx.framework/Headers contains NIDAQmx.h, but this only works on OS X.
NIDAQmx_HEADER_DIR = /Library/Frameworks/nidaqmx.framework/Headers
CFLAGS += -I$(NIDAQmx_HEADER_DIR) -arch i386 -arch x86_64
LDFLAGS += -arch i386 -arch x86_64
endif

all: $(TARGET)

build: $(TARGET) clean_obs

clean:
	rm -f *.o $(TARGET) core client server tsocks

clean_obs:
	rm -f *.o core 

$(TARGET) : $(OBJS) nimeasure
	$(CXX) $(LDFLAGS) $(DEPS) -o $@ $(OBJS)

tsocks: testsockets socketutils
	$(CXX) -Wall -std=c++11 -pthread testsockets.o socketutils.o -o $@

example: socketutils functionapi eventhandler
	$(CXX)  $(LDFLAGS) $(LIBFLAGS) -ggdb3 -Wall -std=c++11 serverexample.cpp *.o  -o server; \
	$(CXX) $(LDFLAGS) $(LIBFLAGS) -Wall -std=c++11 clientexample.cpp *.o  -o client

# serverexample: serverexample.o
# 	$(CXX) -c $@.cpp

# clientexample: clientexample.o
# 	$(CXX) -c $@.cpp

functionapi: functionapi.o
	$(CXX) -c $@.cpp

socketutils: socketutils.o timeutils
	$(CXX) -c $@.cpp

testsockets: testsockets.o
	$(CXX) -c $@.cpp

timeutils: timeutils.o
	$(CXX) -c $@.cpp

eventhandler: eventhandler.o 
	$(CXX) -c $@.cpp
