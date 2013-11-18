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
		"Ydata":Y数据
		"Udata":U数据
		"Vdata":V数据
		"width":视频帧宽度，单位像素
		"height":视频帧高度，单位像素
		"YStride":Y数据行宽，单位字节
		"UVStride":UV数据行宽，单位字节
*/

#endif