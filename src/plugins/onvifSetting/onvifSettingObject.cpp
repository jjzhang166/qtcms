#include "onvifSettingObject.h"
#include <guid.h>
#include <QDebug>
onvifSettingObject::onvifSettingObject():QWebPluginFWBase(this)
{
	for (int i=0;i<THREADNUM;i++)
	{
		onvifSettingRun *pItem=new onvifSettingRun(i);
		connect(pItem,SIGNAL(sgThreadStart(QVariantMap)),this,SLOT(slThreadStart(QVariantMap)));
		connect(pItem,SIGNAL(sgThreadEnd(QVariantMap)),this,SLOT(slThreadEnd(QVariantMap)));
		connect(pItem,SIGNAL(sgThreadInfo(QVariantMap)),this,SLOT(slThreadInfo(QVariantMap)));
		m_tOnvifSettingRunList.append(pItem);
	}
}


onvifSettingObject::~onvifSettingObject()
{
}
void onvifSettingObject::getOnvifDeviceBaseInfo()
{
	int nThreadId;
	if (getFreeThreadId(m_tOnvifDeviceInfo.sIp,GETDEVICEINFO,nThreadId))
	{
		// 信息写入队列
		if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceParam(m_tOnvifDeviceInfo.sIp,m_tOnvifDeviceInfo.sPort,m_tOnvifDeviceInfo.sUserName,m_tOnvifDeviceInfo.sPassword))
		{
			saveOperationItem(m_tOnvifDeviceInfo.sIp,GETDEVICEINFO,nThreadId);
			if (m_tOnvifSettingRunList[nThreadId]->getOnvifDevcieBaseInfo())
			{
				return;
			}else{
				removeOperationItem(m_tOnvifDeviceInfo.sIp,GETDEVICEINFO,nThreadId);
				qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceBaseInfo fail as getOnvifDeviceNetworkInfo fail";
			}	
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceBaseInfo fail as setOnvifDeviceParam fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceBaseInfo fail as getFreeThreadId fail";
	}
	return ;
}
void onvifSettingObject::getOnvifDeviceNetworkInfo()
{
	int nThreadId;
	if (getFreeThreadId(m_tOnvifDeviceInfo.sIp,GETNETWORKINFO,nThreadId))
	{
		// 信息写入队列
		if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceParam(m_tOnvifDeviceInfo.sIp,m_tOnvifDeviceInfo.sPort,m_tOnvifDeviceInfo.sUserName,m_tOnvifDeviceInfo.sPassword))
		{
			saveOperationItem(m_tOnvifDeviceInfo.sIp,GETNETWORKINFO,nThreadId);
			if (m_tOnvifSettingRunList[nThreadId]->getOnvifDeviceNetworkInfo())
			{
				return;
			}else{
				removeOperationItem(m_tOnvifDeviceInfo.sIp,GETNETWORKINFO,nThreadId);
				qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceNetworkInfo fail as getOnvifDeviceNetworkInfo fail";
			}	
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceNetworkInfo fail as setOnvifDeviceParam fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceNetworkInfo fail as getFreeThreadId fail";
	}
	return ;
}

bool onvifSettingObject::setOnvifDeviceNetWorkInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns )
{
	int nThreadId;
	if (getFreeThreadId(m_tOnvifDeviceInfo.sIp,SETNETWORKINFO,nThreadId))
	{
		// 信息写入队列
		if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceParam(m_tOnvifDeviceInfo.sIp,m_tOnvifDeviceInfo.sPort,m_tOnvifDeviceInfo.sUserName,m_tOnvifDeviceInfo.sPassword))
		{
			saveOperationItem(m_tOnvifDeviceInfo.sIp,SETNETWORKINFO,nThreadId);
			if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceNetWorkInfo(m_tOnvifDeviceInfo.sSetIp,m_tOnvifDeviceInfo.sSetMac,m_tOnvifDeviceInfo.sSetGateway,m_tOnvifDeviceInfo.sSetMask,m_tOnvifDeviceInfo.sSetDns))
			{
				return true;
			}else{
				removeOperationItem(m_tOnvifDeviceInfo.sIp,SETNETWORKINFO,nThreadId);
				qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceNetWorkInfo fail as setOnvifDeviceNetWorkInfo fail";
			}	
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceNetWorkInfo fail as setOnvifDeviceParam fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceNetWorkInfo fail as getFreeThreadId fail";
	}
	return false;
}

void onvifSettingObject::getOnvifDeviceEncoderInfo()
{
	int nThreadId;
	if (getFreeThreadId(m_tOnvifDeviceInfo.sIp,GETENCODERINFO,nThreadId))
	{
		// 信息写入队列
		if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceParam(m_tOnvifDeviceInfo.sIp,m_tOnvifDeviceInfo.sPort,m_tOnvifDeviceInfo.sUserName,m_tOnvifDeviceInfo.sPassword))
		{
			saveOperationItem(m_tOnvifDeviceInfo.sIp,GETENCODERINFO,nThreadId);
			if (m_tOnvifSettingRunList[nThreadId]->getOnvifDeviceEncoderInfo())
			{
				return ;
			}else{
				removeOperationItem(m_tOnvifDeviceInfo.sIp,GETENCODERINFO,nThreadId);
				qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceEncoderInfo fail as getOnvifDeviceEncoderInfo fail";
			}	
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceEncoderInfo fail as setOnvifDeviceParam fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getOnvifDeviceEncoderInfo fail as getFreeThreadId fail";
	}
}

bool onvifSettingObject::setOnvifDeviceEncoderInfo( int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile )
{
	int nThreadId;
	if (getFreeThreadId(m_tOnvifDeviceInfo.sIp,SETENCODERINFO,nThreadId))
	{
		// 信息写入队列
		if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceParam(m_tOnvifDeviceInfo.sIp,m_tOnvifDeviceInfo.sPort,m_tOnvifDeviceInfo.sUserName,m_tOnvifDeviceInfo.sPassword))
		{
			saveOperationItem(m_tOnvifDeviceInfo.sIp,SETENCODERINFO,nThreadId);
			if (m_tOnvifSettingRunList[nThreadId]->setOnvifDeviceEncoderInfo(nIndex,nWidth,nHeight,sEnc_fps,sEnc_bps,sCodeFormat,sEncInterval,sEncProfile))
			{
				return true;
			}else{
				removeOperationItem(m_tOnvifDeviceInfo.sIp,SETENCODERINFO,nThreadId);
				qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceEncoderInfo fail as getOnvifDeviceEncoderInfo fail";
			}	
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceEncoderInfo fail as setOnvifDeviceParam fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setOnvifDeviceEncoderInfo fail as getFreeThreadId fail";
	}
	return false;
}

bool onvifSettingObject::setOnvifDeviceParam( QString sIp,QString sPort,QString sUserName,QString sPassword )
{
		m_tOnvifDeviceInfo.sIp=sIp;
		m_tOnvifDeviceInfo.sPort=sPort;
		m_tOnvifDeviceInfo.sUserName=sUserName;
		m_tOnvifDeviceInfo.sPassword=sPassword;
		return true;
}

bool onvifSettingObject::getFreeThreadId( QString sDeviceIp,tagOnvifSettingStepCode tOperationType ,int &nThreadId)
{
	quint64 uiCurrentTime=QDateTime::currentDateTime().toTime_t();
	bool bFlags=true;
	m_tOperationListMutex.lock();
	for (int i=0;i<m_tOperationList.size();i++)
	{
		tagOnvifOperationItem tItem=m_tOperationList.value(i);
		if (tItem.sDeviceIp==sDeviceIp)
		{
			if (tItem.tOperationType==tOperationType)
			{
				if (uiCurrentTime-tItem.uiStartTime<OPERATIONINTERVAL)
				{
					// 禁止重复操作
					qDebug()<<__FUNCTION__<<__LINE__<<"getFreeThreadId as it ban on frequent operation";
					bFlags=false;
					break;
				}else{
					//keep going
				}
			}else{
				//keep going
			}
		}else{
			//keep going
		}
	}
	if (bFlags==false)
	{
		m_tOperationListMutex.unlock();
		return false;
	}else{
		//keep going
	}
	//寻找 空余的线程
	bFlags=false;
	for (int i=0;i<THREADNUM;i++)
	{
		bool bExist=false;
		for (int j=0;j<m_tOperationList.size();j++)
		{
			if (i==m_tOperationList.value(j).nThreadId)
			{
				bExist=true;
				break;
			}else{
				//keep going
			}
		}
		if (bExist==false)
		{
			nThreadId=i;
			bFlags=true;
			break;
		}else{
			//keep going
		}
	}
	m_tOperationListMutex.unlock();
	return bFlags;
}

void onvifSettingObject::saveOperationItem( QString sDeviceIp,tagOnvifSettingStepCode tOperationType,int nThreadId )
{
	m_tOperationListMutex.lock();
	tagOnvifOperationItem tItem;
	tItem.nThreadId=nThreadId;
	tItem.sDeviceIp=sDeviceIp;
	tItem.tOperationType=tOperationType;
	tItem.uiStartTime=QDateTime::currentDateTime().toTime_t();
	m_tOperationList.append(tItem);
	m_tOperationListMutex.unlock();
}

void onvifSettingObject::removeOperationItem( QString sDevice,tagOnvifSettingStepCode tOperationType,int nThreadId )
{
	m_tOperationListMutex.lock();
	bool bFlag=false;
	int n=0;
	for (int i=0;i<m_tOperationList.size();i++)
	{
		if (m_tOperationList.value(i).nThreadId==nThreadId)
		{
			n=i;
			bFlag=true;
			break;
		}else{
			//keep going
		}
	}
	if (bFlag==true)
	{
		m_tOperationList.removeAt(n);
	}else{
		//do nothing
	}
	m_tOperationListMutex.unlock();
}

void onvifSettingObject::slThreadStart( QVariantMap tMap )
{
	// send to ui
	m_tThreadMutex.lock();
	EventProcCall("operationStart",tMap);
	m_tThreadMutex.unlock();
}

void onvifSettingObject::slThreadEnd( QVariantMap tMap )
{
	//remove item for list and send to ui
	m_tThreadMutex.lock();
	EventProcCall("operationEnd",tMap);
	m_tThreadMutex.unlock();
	QString sDeviceIp=tMap.value("ip").toString();
	tagOnvifSettingStepCode tOperationType=(tagOnvifSettingStepCode)tMap.value("operationType").toInt();
	int nThreadId=tMap.value("nThreadId").toInt();
	removeOperationItem(sDeviceIp,tOperationType,nThreadId);
}

void onvifSettingObject::slThreadInfo( QVariantMap tMap )
{
	//send to ui
	m_tThreadMutex.lock();
	EventProcCall("operationReturnInfo",tMap);
	m_tThreadMutex.unlock();
}


