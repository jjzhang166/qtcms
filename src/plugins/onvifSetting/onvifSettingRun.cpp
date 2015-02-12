#include "onvifSettingRun.h"
#include <guid.h>
#include <QDebug>
#include <QDomDocument>
onvifSettingRun::onvifSettingRun(int nThreadId):m_nThreadId(nThreadId),
	m_pOnvifReoteInfo(NULL)
{
	pcomCreateInstance(CLSID_OnvifNetwork,NULL,IID_IOnvifRemoteInfo,(void**)&m_pOnvifReoteInfo);
}


onvifSettingRun::~onvifSettingRun(void)
{
	while(QThread::isRunning()){
		msleep(10);
	}
	if (NULL!=m_pOnvifReoteInfo)
	{
		m_pOnvifReoteInfo->Release();
		m_pOnvifReoteInfo=NULL;
	}
}

void onvifSettingRun::run()
{
	QVariantMap tStartMap;
	tStartMap.insert("nThreadId",m_nThreadId);
	tStartMap.insert("ip",m_tOnvifDeviceInfo.sIp);
	tStartMap.insert("operationType",m_tStepCode);
	emit sgThreadStart(tStartMap);
	bool bFlags=false;
	QString sXmlInfo;
	if (NULL!=m_pOnvifReoteInfo)
	{
		m_pOnvifReoteInfo->setOnvifDeviceInfo(m_tOnvifDeviceInfo.sIp,m_tOnvifDeviceInfo.sPort,m_tOnvifDeviceInfo.sUserName,m_tOnvifDeviceInfo.sPassword);	
	
		switch(m_tStepCode){
		case GETNETWORKINFO:{
			QString sMac;
			QString sGateway;
			QString sMask;
			QString sDns;
			if (m_pOnvifReoteInfo->getOnvifDeviceNetworkInfo(sMac,sGateway,sMask,sDns))
			{
				bFlags=true;
				QDomDocument tDoc;
				QDomElement tItem=tDoc.createElement("NetworkInfo");
				tItem.setAttribute("sMac",sMac);
				tItem.setAttribute("sGateway",sGateway);
				tItem.setAttribute("sMask",sMask);
				tItem.setAttribute("sDns",sDns);
				tDoc.appendChild(tItem);
				sXmlInfo=tDoc.toString();
			}else{
				//do noting
			}
							}
							break;
		case SETNETWORKINFO:{
			if (m_pOnvifReoteInfo->setOnvifDeviceNetWorkInfo(m_tOnvifDeviceInfo.sSetIp,m_tOnvifDeviceInfo.sSetMac,m_tOnvifDeviceInfo.sSetGateway,m_tOnvifDeviceInfo.sSetMask,m_tOnvifDeviceInfo.sSetDns))
			{
				bFlags=true;
			}else{
				//do nothing
			}
							}
							break;
		case GETENCODERINFO:{
			bFlags=true;
			sXmlInfo=m_pOnvifReoteInfo->getOnvifDeviceEncoderInfo();
							}
							break;
		case SETENCODERINFO:{
			if (m_pOnvifReoteInfo->setOnvifDeviceEncoderInfo(m_tOnvifDeviceInfo.nIndex,m_tOnvifDeviceInfo.nWidth,m_tOnvifDeviceInfo.nHeight,m_tOnvifDeviceInfo.sEnc_fps,m_tOnvifDeviceInfo.sEnc_bps,m_tOnvifDeviceInfo.sCodeFormat,m_tOnvifDeviceInfo.sEncInterval,m_tOnvifDeviceInfo.sEncProfile))
			{
				bFlags=true;
			}else{
				//do nothing
			}
							}
							break;
		case GETDEVICEINFO:{
			bFlags=true;
			sXmlInfo=m_pOnvifReoteInfo->getOnvifDeviceInfo();
						   }
						   break;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"operation fail as m_pOnvifReoteInfo is null";
		//do nothing
	}
	QVariantMap tThreadInfo;
	tThreadInfo.insert("nThreadId",m_nThreadId);
	tThreadInfo.insert("ip",m_tOnvifDeviceInfo.sIp);
	tThreadInfo.insert("operationType",m_tStepCode);
	if (bFlags==true)
	{
		tThreadInfo.insert("status",0);
		sXmlInfo.remove("\n");
		tThreadInfo.insert("info",sXmlInfo);
	}else{
		tThreadInfo.insert("status",1);
		tThreadInfo.insert("info",sXmlInfo);
	}
	emit sgThreadInfo(tThreadInfo);

	QVariantMap tEndMap;
	tEndMap.insert("nThreadId",m_nThreadId);
	tEndMap.insert("ip",m_tOnvifDeviceInfo.sIp);
	tEndMap.insert("operationType",m_tStepCode);
	emit sgThreadEnd(tEndMap);
}

bool onvifSettingRun::setOnvifDeviceParam( QString sIp,QString sPort,QString sUserName,QString sPassword )
{
	m_tOnvifDeviceInfo.sIp=sIp;
	m_tOnvifDeviceInfo.sPort=sPort;
	m_tOnvifDeviceInfo.sUserName=sUserName;
	m_tOnvifDeviceInfo.sPassword=sPassword;
	return true;
}

bool onvifSettingRun::getOnvifDeviceNetworkInfo()
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceNetworkInfo fail as the thread is busy";
		return false;
	}else{
		m_tStepCode=GETNETWORKINFO;
		QThread::start();
	}
	return true;
}
bool onvifSettingRun::getOnvifDevcieBaseInfo()
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDevcieBaseInfo fail as the thread is busy";
		return false;
	}else{
		m_tStepCode=GETDEVICEINFO;
		QThread::start();
	}
	return true;
}
bool onvifSettingRun::setOnvifDeviceNetWorkInfo( QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceNetWorkInfo fail as the thread is busy";
		return false;
	}else{
		m_tOnvifDeviceInfo.sSetIp=sSetIp;
		m_tOnvifDeviceInfo.sSetMac=sSetMac;
		m_tOnvifDeviceInfo.sSetGateway=sSetGateway;
		m_tOnvifDeviceInfo.sSetDns=sSetDns;
		m_tOnvifDeviceInfo.sSetMask=sSetMask;
		m_tStepCode=SETNETWORKINFO;
		QThread::start();
	}
	return true;
}

bool onvifSettingRun::getOnvifDeviceEncoderInfo()
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceEncoderInfo fail as the thread is busy";
		return false;
	}else{
		m_tStepCode=GETENCODERINFO;
		QThread::start();
	}
	return true;
}

bool onvifSettingRun::setOnvifDeviceEncoderInfo( int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceEncoderInfo fail as the thread is busy";
		return false;
	}else{
		m_tOnvifDeviceInfo.nIndex=nIndex;
		m_tOnvifDeviceInfo.nWidth=nWidth;
		m_tOnvifDeviceInfo.nHeight=nHeight;
		m_tOnvifDeviceInfo.sEnc_fps=sEnc_fps;
		m_tOnvifDeviceInfo.sEnc_bps=sEnc_bps;
		m_tOnvifDeviceInfo.sCodeFormat=sCodeFormat;
		m_tOnvifDeviceInfo.sEncInterval=sEncInterval;
		m_tOnvifDeviceInfo.sEncProfile=sEncProfile;
		m_tStepCode=SETENCODERINFO;
		QThread::start();
	}
	return true;
}


