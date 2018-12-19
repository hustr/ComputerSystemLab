#!/bin/bash
g++ -o lsdir lsdir.cpp
if [ $? -eq 0 ];
then
    echo 'build finished'
    ./lsdir $1
else
    echo 'build failed'
fi

