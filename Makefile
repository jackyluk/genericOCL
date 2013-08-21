ARCH = $(shell getconf LONG_BIT)
ARCHPATH_32 = x86
ARCHPATH_64 = x86_64
ARCHPATH = $(ARCHPATH_$(ARCH))

CFLAGS = -W -Wall -g -Ihost/include
CXXFLAGS = -W -Wall -g -Ihost/include
CC = gcc
ALL = host device helloworld addition matrix

all: $(ALL)


helloworld: helloworld.o
	g++ -Lhost/build/lib/$(ARCHPATH) -o helloworld helloworld.o -lOpenCL

addition: addition.o
	g++ -Lhost/build/lib/$(ARCHPATH) -o addition addition.o -lOpenCL

matrix: matrix.o
	g++ -Lhost/build/lib/$(ARCHPATH) -o matrix matrix.o -lOpenCL

device: device/device

device/%:
	$(MAKE) -C device/

host: host/build
host/%: 
	$(MAKE) -C host/

%.o: %.cc
	g++ $(CXXFLAGS) -c -L$(NOVELCLSDKROOT)/lib/$(ARCHPATH) -o $@ $<

clean:
	@rm -f *.o helloworld addition matrix
	$(MAKE) -C device/ clean
	$(MAKE) -C host/ clean
