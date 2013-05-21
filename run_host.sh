#!/bin/bash

bin=$1

if [ -z $1 ]
then
    echo "no host binary specified"
    exit 1
else
    LD_LIBRARY_PATH=host/ ./${bin}
fi

