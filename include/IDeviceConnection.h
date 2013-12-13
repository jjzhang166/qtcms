#ifndef __ILOGINDEVICE_HEAD_FILE_ASDNVG8Y9ASDF__
#define __ILOGINDEVICE_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QVariantMap>
#include <QtCore/QString>

interface IDeviceConnection : public IPComBase
{
	//设置host
	//返回值
	//0：设置成功
	//1：设置失败
	virtual int setDeviceHost(const QString & sAddr) = 0;
	//设置Ports
	//返回值
	//0：设置成功
	//1：设置失败
	virtual int setDevicePorts(const QVariantMap & ports) = 0;
	//设置Id
	//返回值
	//0：设置成功
	//1：设置失败
	virtual int setDeviceId(const QString & sAddress) = 0;
	//设置Authority
	//返回值
	//0：设置成功
	//1：设置失败
	virtual int setDeviceAuthorityInfomation(QString username,QString password) = 0;
	//连接设备
	//返回值
	//0：连接成功
	//1：连接失败
	virtual int connectToDevice() = 0;
	// 校验用户名密码
	// 返回值:
	//	0:校验成功
	//	1:校验失败
	virtual int authority() = 0;
	//断开连接
	//返回值
	//0：断开成功
	//1：断开失败
	virtual int disconnect() = 0;
	virtual int getCurrentStatus() = 0;
	virtual QString getDeviceHost() = 0;
	virtual QString getDeviceid() = 0;
	virtual QVariantMap getDevicePorts() = 0;
	enum _enConnectionStatus{
		CS_Disconnected,
		CS_Connectting,
		CS_Connected,
		CS_Disconnecting
	};

};

/*
setDevicePorts:parameter
	ports:
	"media":码流数据端口
*/


#endif