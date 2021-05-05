#!/bin/bash
#
bison -d minipy-lab.y
flex minipy-lab.l
g++ minipy-lab.tab.c -o minipy
./minipy
