#ifndef __IREMOTEBACKUP_HEAD_FILE_OASDF892389HUDSAF80BAS__
#define __IREMOTEBACKUP_HEAD_FILE_OASDF892389HUDSAF80BAS__
#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QDateTime>

interface IRemoteBackup : public IPComBase
{

	// 设置备份参数
	// sAddr:设备ip
	// uiPort:设备端口
	// sEseeId:设备id
	//sDeviceName:设备名称
	// nChannel:最低位为0通道，最高位为31通道，每一位表示一个通道
	// nTypes:录像类型，按位计算，第0位表示定时录像，第1位表示移动侦测录像，第2位表示报警录像，第3位表示手动录像
	// startTime:开始时间
	// endTime:结束时间
	// sbkpath:备份路径

	virtual int startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
							int nChannel,
							int nTypes,
							const QString &sDeviceName,
							const QDateTime & startTime,
							const QDateTime & endTime,
							const QString & sbkpath) = 0;

	virtual int stopBackup() = 0;

	//获取备份进度 （0.0~1.0）
	virtual float getProgress() = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};
};

// event
// 	@1 name "backupEvent"
// parameters:
// 	"types":"startBackup"开始进行备份 "stopBackup"停止备份 "diskfull"磁盘已满 "backupFinished"备份完成 "noStream"获取不到码流
// @2 name "progress" 备份进度
// parameters:
// "parm":" 范围为：0-100；

#endif