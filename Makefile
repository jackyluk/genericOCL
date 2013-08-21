ARCH = $(shell getconf LONG_BIT)
ARCHPATH_32 = x86
ARCHPATH_64 = x86_64
ARCHPATH = $(ARCHPATH_$(ARCH))

CFLAGS = -W -Wall -g -Ihost/include
CXXFLAGS = -W -Wall -g -Ihost/include
CC = gcc
ALL = host device demos

all: $(ALL)

demos: FORCE
	$(MAKE) -C demos

device: device/device

device/%:
	$(MAKE) -C device/

host: host/build
host/%: 
	$(MAKE) -C host/

%.o: %.cc
	g++ $(CXXFLAGS) -c -L$(NOVELCLSDKROOT)/lib/$(ARCHPATH) -o $@ $<

clean:
	$(MAKE) -C demos/ clean
	$(MAKE) -C device/ clean
	$(MAKE) -C host/ clean

FORCE:
