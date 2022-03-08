CC=gcc

all:
	gcc -Wall main.c -o gwm `pkg-config --cflags --libs xcb`

run:
	./gwm
