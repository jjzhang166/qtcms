#pragma once

#include <qwfw.h>
#include <QVariantMap>
#include <IOnvifRemoteInfo.h>
class onvifSettingObject:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	onvifSettingObject();
	~onvifSettingObject();
public slots:
	bool setOnvifDeviceParam(QString sIp,QString sPort,QString sUserName,QString sPassword);
	QVariantMap getOnvifDeviceNetworkInfo();
	bool setOnvifDeviceNetWorkInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns);
	QString getOnvifDeviceEncoderInfo();
	bool setOnvifDeviceEncoderInfo(int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile);
private:
	IOnvifRemoteInfo *m_pIOnvifReoteInfo;
};

