#ifndef __IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__
#define __IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QVariantMap>

//详细接口说明参见doc\Interface\IRecorder.html

interface IRecorder : public IPComBase
{
	//typedef struct _tagFrameInfo{
	//	int type;
	//	char * pData;
	//	unsigned int uiDataSize;
	//	unsigned int uiTimeStamp;
	//}FrameInfo;

	virtual int Start() = 0;

	virtual int Stop() = 0;

	virtual int InputFrame(QVariantMap& frameinfo) = 0;

	virtual int SetDevInfo(const QString& devname,int nChannelNum) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};

/*
	frameinfo参数：
	"frametype":帧类型，取值'I'0x01,'P'0x02,'B','A'0x00
	"data":数据指针
	"length":数据长度
	"pts":当前帧64位时间戳，精确到微妙
	"gentime":帧的产生事件，单位为秒，为GMT时间
	
	"channel":当前帧的通道号，如果是音频帧，不传递该参数
	"width":视频帧的宽，单位像素，如果是音频帧，不传递该参数
	"height":视频帧的高，单位像素，如果是音频帧，不传递该参数
	"vcodec":视频帧的编码格式，当前定义值:"H264"，如果是音频帧，不传递该参数
	
	"samplerate":音频采样率，如果是视频帧，不传递该参数
	"samplewidth":音频采样位宽，如果是视频帧，不传递该参数
	"audiochannel":音频的采样通道数，如果是视频帧，不传递该参数
	"acodec":音频编码格式，当前定义值："G711"，如果是视频帧，不传递该参数
*/


#endif //__IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__