#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "picture_t.h"
#include "simplerecorder.h"

static char osd_string[16];
static void gen_osd_info()
{
	time_t t = time(0);
	strftime(osd_string, 16, "%H:%M:%S",gmtime(&t));
}
int main()
{
	int i;
	struct picture_t pic;
	errno = 0;
	if(!camera_init(&pic))
		goto error;
	if(!output_init())
		goto error_output;
	if(!camera_on())
		goto error_cam_on;

	for(i = 0; i<70; i++){
		camera_get_frame(&pic);
		gen_osd_info();
		osd_print(&pic, osd_string);
		output_write(pic.buffer, pic.width*pic.height*3/2);
	}

	camera_off();
error_cam_on:
	output_close();
error_output:
	camera_close();
error:
	return 0;
}