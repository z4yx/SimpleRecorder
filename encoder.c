#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "MFC_API/SsbSipMfcApi.h"
#include "picture_t.h"

#define DEFAULT_FPS 25

SSBSIP_MFC_ENC_H264_PARAM h264_param;
SSBSIP_MFC_ENC_INPUT_INFO input_info;
SSBSIP_MFC_ENC_OUTPUT_INFO output_info;
void* mfc_handle = NULL;

int Ysize, Csize;
static int64_t start_timestamp;

static void param_init(int width, int height)
{
	h264_param.codecType    = H264_ENC;
	h264_param.SourceWidth  = width;
	h264_param.SourceHeight = height;
	h264_param.IDRPeriod    = 100;
	h264_param.SliceMode    = 0;
	h264_param.RandomIntraMBRefresh = 0;
	h264_param.EnableFRMRateControl = 1;
	h264_param.Bitrate      = 512000;

	h264_param.FrameQp      = 15;
	h264_param.FrameQp_P    = 15;
	h264_param.FrameQp_B    = 20;

	h264_param.QSCodeMax    = 30;
	h264_param.QSCodeMin    = 10;

	h264_param.CBRPeriodRf  = 120;

	h264_param.PadControlOn = 0;             // 0: disable, 1: enable
	h264_param.LumaPadVal   = 0;
	h264_param.CbPadVal     = 0;
	h264_param.CrPadVal     = 0;

	h264_param.ProfileIDC   = 1;
	h264_param.LevelIDC     = 22;
	h264_param.FrameRate    = DEFAULT_FPS * 1000;
	h264_param.SliceArgument = 0;          // Slice mb/byte size number
	h264_param.NumberBFrames = 0;            // 0 ~ 2
	h264_param.NumberReferenceFrames = 2;
	h264_param.NumberRefForPframes   = 2;
	// h264_param.LoopFilterDisable     = 0;    // 1: Loop Filter Disable, 0: Filter Enable
	// h264_param.LoopFilterAlphaC0Offset = 2;
	// h264_param.LoopFilterBetaOffset    = 1;
	h264_param.LoopFilterDisable     = 1;    // 1: Loop Filter Disable, 0: Filter Enable
	h264_param.LoopFilterAlphaC0Offset = 0;
	h264_param.LoopFilterBetaOffset    = 0;
	h264_param.SymbolMode       = 1;         // 0: CAVLC, 1: CABAC
	h264_param.PictureInterlace = 0;
	h264_param.Transform8x8Mode = 1;         // 0: 4x4, 1: allow 8x8
	h264_param.EnableMBRateControl = 0;        // 0: Disable, 1:MB level RC
	h264_param.DarkDisable     = 0;
	h264_param.SmoothDisable   = 0;
	h264_param.StaticDisable   = 0;
	h264_param.ActivityDisable = 0;
	// h264_param.DarkDisable     = 1;
	// h264_param.SmoothDisable   = 1;
	// h264_param.StaticDisable   = 1;
	// h264_param.ActivityDisable = 1;

	h264_param.FrameMap = NV12_LINEAR;
}
static void I420toNV12(unsigned char *pNV12, const unsigned char *pI420, int C_Size)
{
	int halfC = C_Size/2;
	const unsigned char *pCb = pI420;
	const unsigned char *pCr = pI420 + halfC;
	int j;
	for(j=0; j<halfC; j++){
		*pNV12 = *pCb;
		pNV12++;
		*pNV12 = *pCr;
		pNV12++;

		pCr++;
		pCb++;
	}
}
int encoder_init(struct picture_t *info)
{
	SSBIP_MFC_BUFFER_TYPE buf_type = CACHE;

	Ysize = info->width * info->height;
	Csize = Ysize/2;

	param_init(info->width, info->height);

	mfc_handle = SsbSipMfcEncOpen(&buf_type);
	if(!mfc_handle){
		fprintf(stderr, "SsbSipMfcEncOpen failed\n");
		return 0;
	}
	if(SsbSipMfcEncInit(mfc_handle, &h264_param) != MFC_RET_OK){
		fprintf(stderr, "SsbSipMfcEncInit failed\n");
		goto error;
	}

	if (SsbSipMfcEncGetInBuf(mfc_handle, &input_info) != MFC_RET_OK) {
		fprintf(stderr, "SsbSipMfcEncGetInBuf failed\n");
		goto error;
	}
	printf("input_info.YVirAddr=0x%08x, input_info.CVirAddr=0x%08x\n", 
		(uint32_t)input_info.YVirAddr, (uint32_t)input_info.CVirAddr);

	start_timestamp = -1;

	return 1;
	
error:
	SsbSipMfcEncClose(mfc_handle);
	return 0;
}
int encoder_encode_headers(struct encoded_pic_t *headers_out)
{
	if (SsbSipMfcEncGetOutBuf(mfc_handle, &output_info) != MFC_RET_OK) {
		fprintf(stderr, "SsbSipMfcEncGetOutBuf failed\n");
		return 0;
	}
	printf("output_info.OutputVirAddr=0x%08x, output_info.HeaderSize=%d\n", 
		(uint32_t)output_info.StrmVirAddr, (uint32_t)output_info.headerSize);

	headers_out->length = output_info.headerSize;
	headers_out->buffer = output_info.StrmVirAddr;
	return 1;
}
int encoder_encode_frame(struct picture_t *raw_pic, struct encoded_pic_t *output)
{
	int64_t timestamp;
	int timestamp_int;

	timestamp = raw_pic->timestamp.tv_sec*1000000ull+raw_pic->timestamp.tv_usec;
	if(start_timestamp == -1){
		start_timestamp = timestamp;
	}

	timestamp_int = (timestamp-start_timestamp)/1000; // us -> ms
	if (SsbSipMfcEncSetConfig(mfc_handle, MFC_ENC_SETCONF_FRAME_TAG, &(timestamp_int)) != MFC_RET_OK){
		fprintf(stderr, "SsbSipMfcEncSetConfig failed\n");
		return 0;
	}

	memcpy(input_info.YVirAddr, raw_pic->buffer, Ysize);
	I420toNV12(input_info.CVirAddr, raw_pic->buffer+Ysize, Csize);

	if (SsbSipMfcEncSetInBuf(mfc_handle, &input_info) != MFC_RET_OK) {
		fprintf(stderr, "SsbSipMfcEncSetInBuf failed\n");
		return 0;
	}

	if (SsbSipMfcEncExe(mfc_handle) != MFC_RET_OK) {
		fprintf(stderr, "SsbSipMfcEncExe failed\n");
		return 0;
	}

	if (SsbSipMfcEncGetOutBuf(mfc_handle, &output_info) != MFC_RET_OK) {
		fprintf(stderr, "SsbSipMfcEncGetOutBuf failed\n");
		return 0;
	}

	output->buffer = output_info.StrmVirAddr;
	output->length = output_info.dataSize;

	if (SsbSipMfcEncGetConfig(mfc_handle, MFC_ENC_GETCONF_FRAME_TAG, &timestamp_int) != MFC_RET_OK){
		fprintf(stderr, "SsbSipMfcEncGetConfig failed\n");
		return 0;
	}

	output->timepoint = timestamp_int*1000ll;

	if(output_info.frameType == MFC_FRAME_TYPE_P_FRAME){
		applog(", frame_type=P");
		output->frame_type = FRAME_TYPE_P;
	}else if(output_info.frameType == MFC_FRAME_TYPE_I_FRAME){
		applog(", frame_type=I");
		output->frame_type = FRAME_TYPE_I;
	}else{
		applog(", frame_type=B");
		output->frame_type = FRAME_TYPE_B;
	}
	applog(", length=%d, timepoint=%lld", output->length, output->timepoint);

	return 1;
}
void encoder_close()
{
	SsbSipMfcEncClose(mfc_handle);
}
