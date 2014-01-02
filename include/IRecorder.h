#ifndef __IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__
#define __IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>

//详细接口说明参见doc\Interface\IRecorder.html

interface IRecorder : public IPComBase
{
	virtual int Start() = 0;

	virtual int Stop() = 0;

	virtual int InputFrame(int type,char *cbuf,int buffersize) = 0;

	virtual int SetDevInfo(const QString& devname,int nChannelNum) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};

/*
	参数type类型：
	视频帧 0x01(关键帧) 、0x02
	音频帧 0x00

	事件：无
*/


#endif //__IRECORDER_HEAD_FILE_ASDNVG8Y9ASDF__