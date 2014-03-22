#ifndef __IDEVICEREMOTEPLAYBACK_HEAD_FILE_SADVN98ASD7YASGV__
#define __IDEVICEREMOTEPLAYBACK_HEAD_FILE_SADVN98ASD7YASGV__
#include <libpcom.h>
#include <QtGui/QWidget>
#include <QtCore/QDateTime>
// IDeviceGroupRemotePlayback接口用于控制一组通道同步播放

interface IDeviceGroupRemotePlayback : IPComBase
{
	// 将通道添加到同步组，并且指定通道nChannel在窗口wnd内播放
	// 返回值:
	//	0:添加成功
	//	1:组已满，无法添加更多通道
	virtual int AddChannelIntoPlayGroup(int nChannel,QWidget * wnd) = 0;

	// 启动回放
	// nTypes:录像类型，按位计算，第0位表示定时录像，第1位表示移动侦测录像，第2位表示报警录像，第3位表示手动录像
	// startTime:开始时间
	// endTime:结束时间
	// 返回值:
	//	0:调用成功
	//	1:连接中断
	//	2:参数错误
	virtual int GroupPlay(int nTypes,const QDateTime & start,const QDateTime & end) = 0;

	// 获取当前播放时间点
	virtual QDateTime GroupGetPlayedTime() = 0;

	// 暂停播放
	virtual int GroupPause() = 0;

	// 继续播放
	virtual int GroupContinue() = 0;

	// 停止播放
	virtual int GroupStop() = 0;

	// 开启/关闭音频
	// 返回值：返回之前的音频开关状态
	virtual bool GroupEnableAudio(bool bEnable) = 0;

	//设定指定窗口的音量
	virtual int GroupSetVolume(unsigned int uiPersent, QWidget* pWnd) = 0;

	// 快放
	virtual int GroupSpeedFast() = 0;

	// 慢放
	virtual int GroupSpeedSlow() = 0;

	// 恢复正常速度播放
	virtual int GroupSpeedNormal() = 0;
};

#endif