#ifndef __ISETRECORDTIME_HEAD_FILE__0NSDV9FNP9QUAB89A__
#define __ISETRECORDTIME_HEAD_FILE__0NSDV9FNP9QUAB89A__


#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

interface ISetRecordTime : public IPComBase
{
	//修改设备录像时间
	//输入参数：
	//	recordtime_id：指定需修改的录像时间的id号
	//	starttime：开始录像时间
	//	endtime：结束录像时间
	//	enable：true为开始，false为关闭此录像时间
	//返回值：
	//	0：修改成功
	//	1：修改失败
	virtual int ModifyRecordTime(int recordtime_id,QString starttime,QString endtime,bool enable)=0;

	//查询指定通道下的录像时间记录
	//输入参数：
	//	chl_id：通道号
	//返回值：
	//	录像时间记录的id号列表
	virtual QStringList GetRecordTimeBydevId(int chl_id)=0;
	//查询指定录像时间记录条的字段值
	//输入参数：
	//	recordtime_id：录像时间记录id
	//返回值：
	//	录像时间的字段值：
	//		chl_id:通道号
	//		schedle_id：时间段id
	//		weekday：（可选值 0（周一），1（周二），2（周三），3（周四），4（周五），5（周六），6（星期天））
	//		starttime：开始时间
	//		endtime：结束时间
	//		enable：是否使用（0（true），1(false)）
	virtual QVariantMap GetRecordTimeInfo(int recordtime_id)=0;


	enum _enError{
		OK,
		E_SYSTEM_FAILED,
	};
};


#endif