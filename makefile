CC=g++
CFLAGS=-Wall
OSX=./lib_osx
LIN=./lib_linux

all:	

osx: 
	$(CC) -o game -Wall -g  *.cpp -framework SDL2 -I. -L${OSX} -lSDL2_gfx
linux:
	$(CC) -o game -Wall -g  *.cpp `sdl2-config --cflags --libs` -I. -L${LIN} -lSDL2_gfx 
