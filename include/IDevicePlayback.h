#ifndef __IDEVICEREMOTEPLAYBACK_HEAD_FILE_DY3SEX78DFG42LMGK73V__
#define __IDEVICEREMOTEPLAYBACK_HEAD_FILE_DY3SEX78DFG42LMGK73V__
#include <libpcom.h>
#include <QtGui/QWidget>
#include <QtCore/QDateTime>

interface IDeviceRemotePlayback : IPComBase
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

#endif