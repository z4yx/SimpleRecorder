#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "picture_t.h"
#include "simplerecorder.h"

#define OUTPUT_FILENAME "output.yuv"

static int fd;
int output_init()
{
	if((fd = open(OUTPUT_FILENAME, O_WRONLY|O_TRUNC|O_CREAT)) < 0){
		perror("open output file");
		return 0;
	}
	return 1;
}
int output_write(unsigned char *pic, int len)
{
	if(-1 == write(fd, pic, len)) {
		perror("write output file");
		return 0;
	}
	return 1;
}
void output_close()
{
	close(fd);
}