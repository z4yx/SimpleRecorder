SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
CC=arm-none-linux-gnueabi-gcc

all: simplerecorder

simplerecorder: $(OBJS)
	$(CC) -Wall -o $@ $^ -lv4lconvert -lx264
%.o: %.c
	$(CC) -Wall -c $< -o $@
clean:
	rm $(OBJS) simplerecorder
