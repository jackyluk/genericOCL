CFLAGS = -W -Wall -g -I.
CXXFLAGS = -W -Wall -g -I.
CC = gcc
ALL = device helloworld addition matrix

all: $(ALL)


helloworld: helloworld.o host/libfpgaOCL.so
	g++ -Lhost/ -o helloworld helloworld.o -lfpgaOCL

addition: addition.o host/libfpgaOCL.so
	g++ -Lhost/ -o addition addition.o -lfpgaOCL

matrix: matrix.o host/libfpgaOCL.so
	g++ -Lhost/ -o matrix matrix.o -lfpgaOCL

device: device/device

device/%:
	$(MAKE) -C device/

host/%:
	$(MAKE) -C host/

%.o: %.cc
	g++ $(CXXFLAGS) -c -Lhost/ -o $@ $<

clean:
	rm *.o helloworld addition matrix
	$(MAKE) -C device/ clean
	$(MAKE) -C host/ clean
