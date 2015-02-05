#include "onvifSettingObject.h"


onvifSettingObject::onvifSettingObject():QWebPluginFWBase(this)
{
}


onvifSettingObject::~onvifSettingObject()
{
}

QVariantMap onvifSettingObject::getOnvifDeviceNetwordInfo()
{
	QVariantMap tInfo;
	tInfo.insert("sMac","sMacValue");
	tInfo.insert("sGateway","sGatewayValue");
	tInfo.insert("sMask","sMaskValue");
	tInfo.insert("sDns","sDnsValue");
	return tInfo;
}

bool onvifSettingObject::setOnvifDeviceNetWordInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns )
{
	return false;
}

QString onvifSettingObject::getOnvifDeviceEncoderInfo()
{
	return false;
}

bool onvifSettingObject::setOnvifDeviceEncoderInfo( int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile )
{
	return "test";
}

bool onvifSettingObject::setOnvifDeviceParam( QString sIp,QString sPort,QString sUserName,QString sPassword )
{
	return false;
}
