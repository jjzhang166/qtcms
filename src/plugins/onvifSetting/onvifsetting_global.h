#ifndef ONVIFSETTING_GLOBAL_H
#define ONVIFSETTING_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
typedef enum __tagOnvifSettingStepCode{
	GETNETWORKINFO,//获取网络信息
	SETNETWORKINFO,//设置网络信息
	GETENCODERINFO,//获取编码信息
	SETENCODERINFO,//设置编码信息
	GETDEVICEINFO,//获取设备信息
}tagOnvifSettingStepCode;
typedef struct __tagOnvifDeviceInfo{
	QString sIp;
	QString sPort;
	QString sUserName;
	QString sPassword;
	QString sSetIp;
	QString sSetMac;
	QString sSetGateway;
	QString sSetMask;
	QString sSetDns;
	int nIndex;
	int nWidth;
	int nHeight;
	QString sEnc_fps;
	QString sEnc_bps;
	QString sCodeFormat;
	QString sEncInterval;
	QString sEncProfile;
}tagOnvifDeviceInfo;
#endif // ONVIFSETTING_GLOBAL_H
