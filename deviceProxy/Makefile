CC = gcc
ALL = simpleProxy

all: $(ALL)

OBJS=	simpleProxy.o




simpleProxy: $(OBJS)
	$(CC) -o simpleProxy $(OBJS)


.c:
	$(CC) $@.c -o $@	
	

clean:
	rm *.o $(ALL) 
