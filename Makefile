CC=gcc

all:
	$(CC) -Wall main.c -o gwm `pkg-config --cflags --libs x11`

run:
	./gwm

clean:
	rm -f gwm
