#ifndef __ILOGINDEVICE_HEAD_FILE_ASDNVG8Y9ASDF__
#define __ILOGINDEVICE_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QVariantMap>
#include <QtCore/QString>

interface IDeviceConnection : public IPComBase
{
	virtual int setDeviceHost(const QString & sAddr) = 0;
	virtual int setDevicePorts(const QVariantMap & ports) = 0;
	virtual int setDeviceId(const QString & sAddress) = 0;
	virtual int setDeviceAuthorityInfomation(QString username,QString password) = 0;
	virtual int connectToDevice() = 0;
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