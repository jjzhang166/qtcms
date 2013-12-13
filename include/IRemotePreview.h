#ifndef __IPREVIEWCAM_HEAD_FILE_NS89VY9ASUD8__
#define __IPREVIEWCAM_HEAD_FILE_NS89VY9ASUD8__
#include <libpcom.h>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

interface IRemotePreview : public IPComBase
{
	//获取码流
	//返回值
	//0：获取成功
	//1：获取失败
	virtual int getLiveStream(int nChannel, int nStream) = 0;
	//关闭通道
	//返回值
	//0：断开成功
	//1：断开失败
	virtual int stopStream() = 0;
	//暂停码流
	//返回值
	//0：暂停成功
	//1：暂停失败
	virtual int pauseStream(bool bPaused) = 0;
	virtual int getStreamCount() = 0;
	virtual int getStreamInfo(int nStreamId,QVariantMap &streamInfo) = 0;
};

/*
Event:
@1	name: "LiveStream"
	parameters:
		"channel":当前帧的通道号
		"pts":当前帧时间戳，单位为微秒
		"length":数据长度
		"data":数据指针
		"frametype":帧类型，取值'I','P','B','A'
		"width":视频帧的宽，单位像素，如果是音频帧，不传递该参数
		"height":视频帧的高，单位像素，如果是音频帧，不传递该参数
		"vcodec":视频帧的编码格式，当前定义值:"H264"，如果是音频帧，不传递该参数
		"samplerate":音频采样率，如果是视频帧，不传递该参数
		"samplewidth":音频采样位宽，如果是视频帧，不传递该参数
		"audiochannel":音频的采样通道数，如果是视频帧，不传递该参数
		"acodec":音频编码格式，当前定义值："G711"，如果是视频帧，不传递该参数
*/


#endif