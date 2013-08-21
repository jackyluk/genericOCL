ARCH = $(shell getconf LONG_BIT)
ARCHPATH_32 = x86
ARCHPATH_64 = x86_64
ARCHPATH = $(ARCHPATH_$(ARCH))
HOSTPATH = ../../host/build
HOSTPATH_INCLUDE = $(HOSTPATH)/include
HOSTPATH_LIB = $(HOSTPATH)/lib/$(ARCHPATH)

CFLAGS = -W -Wall -g -I$(HOSTPATH_INCLUDE)
CXXFLAGS = -W -Wall -g -I$(HOSTPATH_INCLUDE)
CC = gcc
ALL = helloworld

all: $(ALL)


helloworld: helloworld.o
	g++ -L$(HOSTPATH_LIB) -o helloworld helloworld.o -lOpenCL

%.o: %.cc
	g++ $(CXXFLAGS) -c -L$(HOSTPATH_LIB) -o $@ $<

clean:
	@rm -f *.o helloworld 
