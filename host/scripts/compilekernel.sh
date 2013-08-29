#!/bin/bash

# build an OpenCL kernel using gcc

gcc -fPIC -Wno-implicit-function-declaration -g -x c -O2 -std=c99 -c kernelwrapper.c -o kernelwrapper.o
gcc -fPIC -shared -Wno-implicit-function-declaration -g -O2 -o kernel.so kernelwrapper.o program.o
rm kernelwrapper.c kernelargs
exit 0

