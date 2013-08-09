SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
CC=gcc

all: simplerecorder

simplerecorder: $(OBJS)
	$(CC) -Wall -o $@ $^ -lv4lconvert -lx264
matroska_ebml.o: matroska_ebml.c
	$(CC) -Wall -std=c99 -c $< -o $@
%.o: %.c
	$(CC) -Wall -c $< -o $@
clean:
	rm $(OBJS) simplerecorder