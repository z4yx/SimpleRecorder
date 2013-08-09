#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <x264.h>
#include "picture_t.h"

#define DEFAULT_FPS 25

static x264_t *encoder;
static x264_picture_t pic_in, pic_out;
static int64_t start_timestamp;

int encoder_init(struct picture_t *info)
{
	x264_param_t param;

	if(x264_param_default_preset(&param, "medium", "")<0)
		return 0;
	param.i_threads = 2;
	param.i_width = info->width;
	param.i_height = info->height;
	param.i_fps_num = DEFAULT_FPS;
	param.i_fps_den = 1;
	param.i_frame_reference = 3;
	param.i_bframe = 2;
	param.i_bframe_adaptive = 2;
	param.i_bframe_pyramid = 2;
	param.i_keyint_max = 250;
 
	param.b_annexb = 1;
 
	if(x264_param_apply_profile(&param, "high")<0)
		return 0;

	if(!(encoder = x264_encoder_open(&param)))
		return 0;

	// refer to x264_picture_alloc() in common.c
	x264_picture_init(&pic_in);
    pic_in.img.i_csp = X264_CSP_I420;
    pic_in.img.i_plane = 3;
    pic_in.img.i_stride[0] = info->width;   //Y
    pic_in.img.i_stride[1] = info->width/2; //U
    pic_in.img.i_stride[2] = info->width/2; //V

    start_timestamp = -1;

	return 1;
}
int encoder_encode_headers(struct encoded_pic_t *headers_out)
{
	// SPS/PPS/SEI
	x264_nal_t *headers;
	int i_nal;

	if(x264_encoder_headers( encoder, &headers, &i_nal ) < 0){
		fprintf(stderr,"x264_encoder_headers failed\n");
		return 0;
	}

	// refer to write_headers() in raw.c
	headers_out->length = headers[0].i_payload + headers[1].i_payload + headers[2].i_payload;
	headers_out->buffer = headers[0].p_payload;
	return 1;
}
int encoder_encode_frame(struct picture_t *raw_pic, struct encoded_pic_t *output)
{
	x264_nal_t *nal;
	int i_nal;
	int i_frame_size = 0;
	int64_t timestamp;

	timestamp = raw_pic->timestamp.tv_sec*1000000ull+raw_pic->timestamp.tv_usec;
	if(start_timestamp == -1){
		start_timestamp = timestamp;
	}

	pic_in.i_pts = timestamp-start_timestamp;
	pic_in.img.plane[0] = raw_pic->buffer;
	pic_in.img.plane[1] = pic_in.img.plane[0] + raw_pic->width * raw_pic->height;
	pic_in.img.plane[2] = pic_in.img.plane[1] + raw_pic->width * raw_pic->height / 4;

	i_frame_size = x264_encoder_encode( encoder, &nal, &i_nal, &pic_in, &pic_out );

	if(i_frame_size < 0){
		fprintf(stderr,"x264_encoder_encode failed\n");
		return 0;
	}

	if( i_frame_size )
	{
		output->buffer = nal[0].p_payload;
		output->length = i_frame_size;
		output->timepoint = pic_out.i_pts;
		if(pic_out.i_type == X264_TYPE_P){
			applog(", frame_type=P");
			output->frame_type = FRAME_TYPE_P;
		}else if(IS_X264_TYPE_B(pic_out.i_type)){
			applog(", frame_type=B");
			output->frame_type = FRAME_TYPE_B;
		}else{
			applog(", frame_type=I");
			output->frame_type = FRAME_TYPE_I;
		}
		applog(", timepoint=%lld", output->timepoint);
	}else{
		output->length = 0;
	}
	return 1;
}
void encoder_release(struct encoded_pic_t *output)
{

}
void encoder_close()
{
	x264_encoder_close(encoder);
}
