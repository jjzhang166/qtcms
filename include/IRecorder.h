#ifndef __IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__
#define __IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>

//详细接口说明参见doc\Interface\IRecorder.html

interface IRecorder : public IPComBase
{
	virtual int Start() = 0;

	virtual int Stop() = 0;

	virtual int InputFrame(int type,char *cbuf,int buffersize) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};

/*
	参数type类型：
	REC_SYS_DATA 0x11 为信息数据
	0x1、0x2为视频帧

	事件：无
*/


#endif //__IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__