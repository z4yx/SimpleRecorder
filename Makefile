SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE)) MFC_API/SsbSipMfcEncAPI.o
CC=arm-none-linux-gnueabi-gcc

all: simplerecorder

simplerecorder: $(OBJS)
	$(CC) -Wall -o $@ $^ -lv4lconvert -ljpeg -lm -lrt -L. -static
%.o: %.c
	$(CC) -Wall -c $< -o $@
clean:
	rm $(OBJS) simplerecorder
