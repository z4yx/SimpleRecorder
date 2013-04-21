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

#define OUTPUT_FILENAME "output.h264"

static int fd;
int output_init()
{
	if((fd = open(OUTPUT_FILENAME, O_WRONLY|O_TRUNC|O_CREAT)) < 0){
		perror("open output file");
		return 0;
	}
	return 1;
}
int output_write_headers(struct encoded_pic_t *headers)
{
	if(-1 == write(fd, headers->buffer, headers->length)) {
		perror("write output file");
		return 0;
	}
	return 1;
}
int output_write_frame(struct encoded_pic_t *encoded)
{
	if(-1 == write(fd, encoded->buffer, encoded->length)) {
		perror("write output file");
		return 0;
	}
	return 1;
}
void output_close()
{
	close(fd);
}