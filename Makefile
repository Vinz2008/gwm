CC=gcc
CFLAGS=-Wall $(shell pkg-config --cflags x11) -c -g
LDFLAGS=$(shell pkg-config --libs x11)

SRCS := $(wildcard src/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: gwm
	
gwm: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $^

run:
	./gwm

clean:
	rm -f gwm src/*.o
