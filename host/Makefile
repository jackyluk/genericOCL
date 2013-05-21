CFLAGS = -W -Wall -Wno-unused-parameter -fPIC -g -DSHOWDEBUG
CC = gcc
ALL = dev_interface_test libfpgaOCL.so
OCL_OBJ = cl_platform.o cl_device.o cl_context.o cl_cqueue.o cl_mem.o cl_program.o cl_kernel.o cl_event.o
CFLAGS += -I../
all: $(ALL)

install: libfpgaOCL.so
	cp libfpgaOCL.so /usr/lib/

hostcli: hostcli.o
	$(CC) -c $(CFLAGS) -o $@ hostcli.o


DEV_IF_TEST_OBJ=	dev_interface_test.o\
			dev_interface.o
	

dev_interface_test: $(DEV_IF_TEST_OBJ)
	$(CC) -c $(CFLAGS) -o $@ $(DEV_IF_TEST_OBJ)

libfpgaOCL.so: dev_interface.o $(OCL_OBJ)
	$(CC) $(CFLAGS) -shared -o libfpgaOCL.so $(OCL_OBJ) dev_interface.o -lpthread

# 
# dev_interface.o: dev_interface.c dev_interface.h

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm *.o $(ALL) 
