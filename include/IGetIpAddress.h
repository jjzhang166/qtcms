#ifndef __ILOGINDEVICE_HEAD_FILE_ASSDFDDNVG8Y9ASDF__
#define __ILOGINDEVICE_HEAD_FILE_ASSDFDDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QVariantMap>
#include <QtCore/QString>

interface IGetIpAddress : public IPComBase
{
	//通过设备ID获取设备的通信地址和端口
	//参数：
	//sId:设备id
	//sIp：返回的设备ip
	//sPort：返回的设备端口
	//返回值
	//true：成功
	//flase：失败
	virtual bool getIpAddressEx(const QString sId,QString &sIp,QString &sPort,QString &sHttp) = 0;
};


#endif