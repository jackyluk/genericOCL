CFLAGS= -g -I../ -O2 -DENABLE_THREAD_POOL
CC = gcc
ALL = device

all: $(ALL)

DEVICE_OBJS=	SocketConnector.o \
		ControlLink.o \
		TPScheduler.o \
		ComputeUnit.o \
		Device.o \
		core.o




device: $(DEVICE_OBJS)
	g++ -g -o device $(DEVICE_OBJS) -lpthread -ldl


.cpp:
	$(CC) $(CFLAGS) $@.cpp $(LDFLAGS) -o $@	
	
%.o:%.cpp
	g++ -c $(CFLAGS) $< $(LDFLAGS) -o $@	

clean:
	rm -f *.o kernel.so $(ALL) 
