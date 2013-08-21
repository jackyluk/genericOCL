ARCH = $(shell getconf LONG_BIT)
ARCHPATH_32 = x86
ARCHPATH_64 = x86_64
ARCHPATH = $(ARCHPATH_$(ARCH))
LIBPATH=build/lib/$(ARCHPATH)


CFLAGS = -W -Wall -Wno-unused-parameter -fPIC -g
CC = gcc
ALL = $(LIBPATH)/libOpenCL.so
OCL_OBJ = cl_platform.o cl_device.o cl_context.o cl_cqueue.o cl_mem.o cl_program.o cl_kernel.o cl_event.o logger.o
CFLAGS += -I./include/



all: $(ALL)

install: $(LIBPATH)/libOpenCL.so
	cp $< /usr/lib/

hostcli: hostcli.o
	$(CC) -c $(CFLAGS) -o $@ hostcli.o

build: 
	@mkdir -p $@

build/scripts: build
	@mkdir -p $@
	@cp -R scripts/* build/scripts

build/include:
	@mkdir -p $@
	@cp -R include/* build/include

$(LIBPATH)/: 
	@mkdir -p $@

$(LIBPATH)/libOpenCL.so: dev_interface.o $(OCL_OBJ) build/include build/scripts $(LIBPATH)
	@echo "  AR $@"
	@$(CC) $(CFLAGS) -shared -o $@ $(OCL_OBJ) dev_interface.o -lpthread
	@echo ""
	@echo ""
	@echo ""
	@echo "Please export environment variables:"
	@echo "export NOVELCLSDKROOT=$(CURDIR)/"
	@export NOVELCLSDKROOT=$(CURDIR)/
	@echo "export LD_LIBRARY_PATH=$$()LD_LIBRARY_PATH:$(CURDIR)/$(LIBPATH)"
	@export LD_LIBRARY_PATH=$$()LD_LIBRARY_PATH:$(CURDIR)/$(LIBPATH)

# 
# dev_interface.o: dev_interface.c dev_interface.h

%.o: %.c
	@echo "  CC $@"
	@$(CC) -c $(CFLAGS) -o $@ $<


clean:
	@rm -rf build
	@rm -f *.o $(ALL) 
