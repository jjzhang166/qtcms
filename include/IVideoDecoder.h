#ifndef __IPREVIEWWINDOW_HEAD_FILE_NASV9123400SDV__
#define __IPREVIEWWINDOW_HEAD_FILE_NASV9123400SDV__
#include <libpcom.h>

interface IVideoDecoder : public IPComBase
{
	virtual int init(int nWidth,int nHeight) = 0;
	virtual int deinit() = 0;
	virtual int decode(char * pData,unsigned int nDataLength) = 0;
	virtual int flushDecoderBuffer() = 0;
};

/*
Event:
@1	name: "DecodedFrame"
	parameters:
		”flags“:可用参数标志，按位计算，当对应位置位时，表示该参数可用，否则参数不可用
				0x00000001			DF_MASK_DATA
				0x00000002			DF_MASK_YDATA
				0x00000004			DF_MASK_UDATA
				0x00000008			DF_MASK_VDATA
				0x00000010			DF_MASK_WIDTH
				0x00000020			DF_MASK_HEIGHT
				0x00000040			DF_MASK_YSTRIDE
				0x00000080			DF_MASK_UVSTRIDE
				0x00000100			DF_MASK_LINESTRIDE
				0x00000200			DF_MASK_PIXELFOMAT
		"data":数据地址
		"Ydata":视频Y数据地址
		"Udata":视频U数据地址
		“Vdata":视频V数据地址
		"width":视频帧宽度，单位像素
		"height":视频帧高度，单位像素
		"YStride":Y数据行宽，单位字节
		"UVStride":UV数据行宽，单位字节
		"lineStride":行宽，当像素格式为RGB或者YUV打包格式时使用
		"pixelFormat":像素格式，YUV420_P-"YV12"
*/

#endif