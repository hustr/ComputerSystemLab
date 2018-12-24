#!/bin/bash
g++ main.cpp -o main
echo "build main finished"
g++ readbuf.cpp -o readbuf
echo "build readbuf finished"
g++ writebuf.cpp -o writebuf
echo "build writebuf finished"
echo "build all"
