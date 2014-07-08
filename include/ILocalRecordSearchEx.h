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
	输入参数sTypeList：为检索的类型组合 (格式为"x;x;x" ，例如"0;1;2;3"，检索所有类型的组合)
	0表示定时录像，1表示移动录像，2表示报警录像，3表示手动录像
	--*/
	virtual int searchVideoFileEx(const QString &sDevName,
								const QString& sDate,
								const QString& sTypeList) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};
};



#endif