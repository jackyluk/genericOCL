#!/bin/bash

# build an OpenCL kernel using gcc

gcc -fPIC -Wno-implicit-function-declaration -g -x c -std=c99 -c kernelwrapper.c -o kernelwrapper.o
gcc -fPIC -shared -Wno-implicit-function-declaration -g -o kernel.so kernelwrapper.o program.o
rm kernelwrapper.c kernelargs
exit 0

