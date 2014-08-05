#ifndef __IRECORDEREX_HEAD_FILE_IO34ERG75TF__
#define __IRECORDEREX_HEAD_FILE_IO34ERG75TF__
#include <libpcom.h>
#include <QtCore/QVariantMap>

//详细接口说明参见doc\Interface\IRecorderEx.html

interface IRecorderEx : public IPComBase
{

	virtual int SetDevInfoEx(const int &nWindId, const int &nRecordType) = 0;

	virtual int FixExceptionalData() = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};



#endif //__IRECORDEREX_HEAD_FILE_IO34ERG75TF__