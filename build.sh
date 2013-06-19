#!/bin/sh

mkdir -p bin
g++ src/main.cpp src/serial.c -lcgicc -o bin/hc001-web
