SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
CC=arm-none-linux-gnueabi-gcc
CPP=arm-none-linux-gnueabi-g++

all: simplerecorder

simplerecorder: $(OBJS)
	$(CPP) -Wall -o $@ $^ -static -pthread -L. -lv4lconvert -lm -lrt ./linux_lib/libcedarv_osal.a ./linux_lib/libcedarxalloc.a ./linux_lib/libh264enc.a ./linux_lib/libcedarv.a
%.o: %.c
	$(CC) -Wall -c $< -o $@
clean:
	rm $(OBJS) simplerecorder