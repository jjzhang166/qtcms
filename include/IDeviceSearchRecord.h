#ifndef __IDEVICESEARCHRECORD_HEAD_FILE_ASD89A8SDBV07SADY__
#define __IDEVICESEARCHRECORD_HEAD_FILE_ASD89A8SDBV07SADY__
#include <libpcom.h>

interface IDeviceSearchRecord : IPComBase
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

#endif