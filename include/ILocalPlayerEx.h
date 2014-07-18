#ifndef __ILOCALPLAYEREX_HEAD_FILE_TG7RJ8H5YHTYF__
#define __ILOCALPLAYEREX_HEAD_FILE_TG7RJ8H5YHTYF__
#include <libpcom.h>
#include <QtCore/QVariantMap>

//详细接口说明参见doc\Interface\ILocalPlayerEx.html

interface ILocalPlayerEx : public IPComBase
{

	virtual int AddFileIntoPlayGroupEx(const int & nWndId,
									const QWidget * pWnd,
									const QDate& date,
									const QTime & startTime,
									const QTime & endTime,
									const int & nTypes) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};



#endif //__ILOCALPLAYEREX_HEAD_FILE_TG7RJ8H5YHTYF__