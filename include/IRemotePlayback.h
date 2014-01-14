#ifndef __IREMOTEPLAYBACK_HEAD_FILE_OASDF892389HUDSAF80BAS__
#define __IREMOTEPLAYBACK_HEAD_FILE_OASDF892389HUDSAF80BAS__
#include <libpcom.h>

interface IRemotePlayback : public IPComBase
{
	// 开始搜索录像文件
	// nChannel:最低位为0通道，最高位为31通道，每一位表示一个通道
	// nTypes:录像类型，按位计算，第0位表示定时录像，第1位表示移动侦测录像，第2位表示报警录像，第3位表示手动录像
	// startTime:开始时间
	// endTime:结束时间
	// 返回值:
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	virtual int startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime) = 0;

	// 通过录像的起止时间来获取码流
	// nChannel:最低位为0通道，最高位为31通道，每一位表示一个通道
	// nTypes:录像类型，按位计算，第0位表示定时录像，第1位表示移动侦测录像，第2位表示报警录像，第3位表示手动录像
	// startTime:开始时间
	// endTime:结束时间
	// 返回值:
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	virtual int getPlaybackStreamByTime(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime) = 0;

	// 通过录像文件名来获取码流
	// nChannel:通道号
	// sFileName:录像文件名
	// 返回值
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	virtual int getPlaybackStreamByFileName(int nChannel,const QString &sFileName) = 0;

	// 通知暂停回放码流
	// bPause:true-暂停码流，false-恢复码流
	// 返回值:
	//	0:调用成功
	//	1:调用失败
	virtual int pausePlaybackStream(bool bPause) = 0;

	// 停止当前回放码流
	// 返回值:
	//	0:调用成功
	//	1:调用失败
	virtual int stopPlaybackStream() = 0;

};

// event
// 	@1 name "foundFile"
// parameters:
// 	"channel":录像所属的通道号
// 	"types":录像类型，按位计算，第0位表示定时录像，第1位表示移动侦测录像，第2位表示报警录像，第3位表示手动录像
// 	"start":录像开始时间，格式为"YYYY-MM-DD hh:mm:ss"
// 	"end":录像结束时间，格式为"YYYY-MM-DD hh:mm:ss"
// 	"filename":录像的文件名
// 
// 	@2 name "recFileSearchFinished"
// parameters:
// 	"total":总共的录像记录条目数

// 	@3 name "recStream"
// parameters:
// 	"length":码流裸数据长度
// 	"frametype":帧类型，0为音频，1为视频I帧，2为视频P帧
// 	"channel":当前数据帧的通道号，从0开始计算
// 	"width":视频帧的宽，如果是音频帧则忽略该参数
// 	"height":视频帧的高，如果是音频帧则忽略该参数
// 	"framerate":视频帧的帧率，如果是音频帧则忽略该参数
// 	"audioSampleRate":音频帧的采样率，如果是视频帧则忽略该参数
// 	"audioFormat":音频帧的编码格式，采用字符串表示，如"g711"等
// 	"audioDataWidth":音频帧的采样位宽，如果是视频帧则忽略该参数
// 	"pts":64位时间戳，精确到微秒
// 	"gentime":帧的产生事件，单位为秒，为GMT时间
// 	"data":码流数据的地址


#endif