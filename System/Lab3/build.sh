#!/bin/bash
g++ main.cpp -o main
echo "build main finished"
g++ put.cpp -o put
echo "build put finished"
g++ get.cpp -o get
echo "build get finished"
echo "build all"

