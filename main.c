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

static char osd_string[20];
static void gen_osd_info()
{
	time_t t = time(0);
	strftime(osd_string, 20, "%Y-%m-%d %H:%M:%S",localtime(&t));
}
int main()
{
	int i;
	struct picture_t pic;
	struct encoded_pic_t encoded_pic;
	errno = 0;
	if(!camera_init(&pic))
		goto error_cam;
	if(!encoder_init(&pic)){
		fprintf(stderr,"failed to initialize encoder\n");
		goto error_encoder;
	}
	if(!preview_init(&pic))
		goto error_preview;
	if(!output_init())
		goto error_output;
	if(!encoder_encode_headers(&encoded_pic))
		goto error_output;
	if(!output_write_headers(&encoded_pic))
		goto error_output;
	if(!camera_on())
		goto error_cam_on;

	for(i = 0; i<200; i++){
		if(!camera_get_frame(&pic))
			break;
		gen_osd_info();
		osd_print(&pic, osd_string);
		if((i&7)==0) // i%8==0
			preview_display(&pic);
		if(!encoder_encode_frame(&pic, &encoded_pic))
			break;
		applog_flush();
		if(!output_write_frame(&encoded_pic))
			break;
	}
	putchar('\n');

	camera_off();
error_cam_on:
	output_close();
error_output:
	preview_close();
error_preview:
	encoder_close();
error_encoder:
	camera_close();
error_cam:
	return 0;
}
