#ifndef __ISWITCHSTREAM_HEAD_FILE_V898UY19ASYVB__
#define __ISWITCHSTREAM_HEAD_FILE_V898UY19ASYVB__
#include <libpcom.h>

interface ISwitchStream : public IPComBase
{

	//切换码流
	//StreamNum:码流号，1：代表次码流，0：代表主码流
	//返回值：
	//0:切换成功
	//1:切换失败
	virtual int SwitchStream(int StreamNum)=0;

};

#endif