#!/bin/bash

cmake --build build/

./build/mycompiler input.txt 

./out

echo $?