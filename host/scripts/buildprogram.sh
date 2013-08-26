#!/bin/bash

# build an OpenCL kernel using gcc

src=$1
target=$1.o

if [ -z $1 ]
then
    echo "no argument specified"
    exit 1

elif [ $1=~.*".cl" -a -e $1 ] #if file exists and matches .cl
then
    if [ $1 -ot ${target} ] #check if source is older than target
    then
        echo "nothing to do"
        exit 0
    else
        source tmp.options
        ( echo -e "#include \"kernel.h\""; cat ${src} ) > ${src}.tmp #append header to source
        gcc -I$NOVELCLSDKROOT/include -fPIC -Wno-implicit-function-declaration ${CLFLAGS} -g -x c -std=c99 -c -o ${target} ${src}.tmp
        mv ${target} program.o
        exit 0
    fi
else
    echo "incorrect argument"
    exit 1
fi
