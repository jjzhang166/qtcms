#ifndef __ILOCALPLAYSEARCHEX_HEAD_FILE_FSJ48J8KY4YU5D__
#define __ILOCALPLAYSEARCHEX_HEAD_FILE_FSJ48J8KY4YU5D__
#include <libpcom.h>
#include <QtCore/QString>

interface ILocalRecordSearchEx : public IPComBase
{
	/*++
	搜索符合条件的视频文件。检索结果以事件的形式抛出。
	输入参数sDevName：为设备名
	输入参数sDate：为要搜索的日期 (格式"yyyy-MM-dd")
	输入参数nTypes：为检索的类型组合 (按位计算，第0位表示定时录像，第1位表示移动侦测录像，第2位表示报警录像，第3位表示手动录像，例如"15"，表示检索所有类型的组合)
	--*/
	virtual int searchVideoFileEx(const QString &sDevName,
								const QString& sDate,
								const int& nTypes) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};
};



#endif