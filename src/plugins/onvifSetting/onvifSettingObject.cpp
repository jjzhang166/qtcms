#include "onvifSettingObject.h"
#include <guid.h>
#include <QDebug>
onvifSettingObject::onvifSettingObject():QWebPluginFWBase(this),
	m_pIOnvifReoteInfo(NULL)
{
	pcomCreateInstance(CLSID_OnvifNetwork,NULL,IID_IOnvifRemoteInfo,(void**)&m_pIOnvifReoteInfo);
}


onvifSettingObject::~onvifSettingObject()
{
	if (m_pIOnvifReoteInfo!=NULL)
	{
		m_pIOnvifReoteInfo->Release();
		m_pIOnvifReoteInfo=NULL;
	}
}

QVariantMap onvifSettingObject::getOnvifDeviceNetworkInfo()
{
	QVariantMap tInfo;
	QString sMac;
	QString sGateway;
	QString sMask;
	QString sDns;
	if (NULL!=m_pIOnvifReoteInfo)
	{
		if (m_pIOnvifReoteInfo->getOnvifDeviceNetworkInfo(sMac,sGateway,sMask,sDns))
		{
			tInfo.insert("sMac",sMac);
			tInfo.insert("sGateway",sGateway);
			tInfo.insert("sMask",sMask);
			tInfo.insert("sDns",sDns);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"get networkInfo fail as getOnvifDeviceNetworkInfo fail";
		}
	}
	return tInfo;
}

bool onvifSettingObject::setOnvifDeviceNetWorkInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns )
{
	if (NULL!=m_pIOnvifReoteInfo)
	{
		if (m_pIOnvifReoteInfo->setOnvifDeviceNetWorkInfo(sSetIp,sSetMac,sSetGateway,sSetMask,sSetDns))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"set fail as the under Function return fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"set fail as m_pIOnvifReoteInfo is null";
	}
	return false;
}

QString onvifSettingObject::getOnvifDeviceEncoderInfo()
{
	QString sEncoderInfo;
	if (NULL!=m_pIOnvifReoteInfo)
	{
		sEncoderInfo=m_pIOnvifReoteInfo->getOnvifDeviceEncoderInfo();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"get DeviceEncoderInfo fail as m_pIOnvifReoteInfo is null";
	}
	return sEncoderInfo;
}

bool onvifSettingObject::setOnvifDeviceEncoderInfo( int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile )
{
	if (NULL!=m_pIOnvifReoteInfo)
	{
		if (m_pIOnvifReoteInfo->setOnvifDeviceEncoderInfo(nIndex,nWidth,nHeight,sEnc_fps,sEnc_bps,sCodeFormat,sEncInterval,sEncProfile))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"set EncoderInfo fail as under Function return fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"set EncoderInfo fail as m_pIOnvifReoteInfo is null";
	}
	return false;
}

bool onvifSettingObject::setOnvifDeviceParam( QString sIp,QString sPort,QString sUserName,QString sPassword )
{
	if (NULL!=m_pIOnvifReoteInfo)
	{
		m_pIOnvifReoteInfo->setOnvifDeviceInfo(sIp,sPort,sUserName,sPassword);
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"set device param fail as m_pIOnvifReoteInfo is null";
	}
	return false;
}
