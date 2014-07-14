#include "rPlayBackRunEx.h"


rPlayBackRunEx::rPlayBackRunEx(void):m_bStop(false),
	m_nSleepSwitch(0),
	m_pDeviceGroupRemotePlayBack(NULL)
{
	m_sVendorList<<"DVR"<<"IPC"<<"NVR";
	m_tEventNameList<<"foundFile"<<"recFileSearchFail"<<"CurrentStatus"<<"recFileSearchFinished"<<"bufferStatus";
}


rPlayBackRunEx::~rPlayBackRunEx(void)
{
}

void rPlayBackRunEx::run()
{
	int nStep=RECDEFAULT;
	bool bRecRunStop=false;
	m_bStop=false;
	while(bRecRunStop==false){
		if (nStep!=RECEND&&nStep!=RECGROUPSTOP)
		{
			if (m_qStepCode.isEmpty())
			{
				nStep=RECDEFAULT;
			}else{
				nStep=m_qStepCode.dequeue();
			}
		}else{
			//keep going
		}
		switch(nStep){
		case RECSEARCHFILE:{
			//搜索远程文件

			//创建设备组件
			//组成回调函数
			//设置搜索参数
			//开始搜索
			//搜索成功
			//搜索失败
			//释放资源
			int nSearchStep=0;
			bool bSearchStop=false;
			IDeviceGroupRemotePlayback *pDeviceGroupRemotePlayBack=NULL;
			while(bSearchStop==false){
				switch(nSearchStep){
				case 0:{
					//创建设备组件
					if (getDeviceObject())
					{
						//keep going 
						nSearchStep=1;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"search remote file fail as getDeviceObject fail";
						nSearchStep =4;
					}
					   }
					   break;
				case 1:{
					//组成回调函数
					//设置搜索参数
					IEventRegister *pEventRegister=NULL;
					m_pDeviceGroupRemotePlayBack->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
					if (NULL!=pEventRegister)
					{
						pEventRegister->registerEvent("foundFile",cbXRecRunFoundFile,this);
						pEventRegister->registerEvent("recFileSearchFail",cbXRecRunFileSearchFail,this);
						pEventRegister->registerEvent("CurrentStatus",cbXRecRunStateChange,this);
						pEventRegister->registerEvent("recFileSearchFinished",cbXRecRunFileSearchFinished,this);
						pEventRegister->registerEvent("bufferStatus",cbXRecRunCacheState,this);
						pEventRegister->Release();
						pEventRegister=NULL;
						if (!m_tRecDeviceInfo.sSearchStartTime.isEmpty()&&!m_tRecDeviceInfo.sSearchEndTime.isEmpty())
						{
							IDeviceClient *m_pDeviceClient=NULL;
							m_pDeviceGroupRemotePlayBack->QueryInterface(IID_IDeviceClient,(void**)&m_pDeviceClient);
							if (NULL!=m_pDeviceClient)
							{
								m_pDeviceClient->checkUser(m_tRecDeviceInfo.sUserName,m_tRecDeviceInfo.sPassword);
								m_pDeviceClient->setDeviceHost(m_tRecDeviceInfo.sAddress);
								m_pDeviceClient->setDeviceId(m_tRecDeviceInfo.sEsee);
								m_pDeviceClient->setDevicePorts(m_tRecDeviceInfo.uiPort);
								m_pDeviceClient->Release();
								m_pDeviceClient=NULL;
								nSearchStep=2;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"search remote file fail as it do not support IDeviceClient interface";
								nSearchStep=4;
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"search remote file fail as the starTime or endTime is empty";
							nSearchStep=4;
						}

					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"search remote file fail as it do not support IEventRegister interface";
						nSearchStep =4;
					}
					   }
					   break;
				case 2:{
					//开始搜索
					IDeviceSearchRecord *pDeviceSearchRecord=NULL;
					m_pDeviceGroupRemotePlayBack->QueryInterface(IID_IDeviceSearchRecord,(void**)&pDeviceSearchRecord);
					if (NULL!=pDeviceSearchRecord)
					{
						QDateTime tStart=QDateTime::fromString(m_tRecDeviceInfo.sSearchStartTime,"yyyy-MM-dd hh:mm:ss");
						QDateTime tEnd=QDateTime::fromString(m_tRecDeviceInfo.sSearchEndTime,"yyyy-MM-dd hh:mm:ss");
						if (0==pDeviceSearchRecord->startSearchRecFile(m_tRecDeviceInfo.nSearchChannel,m_tRecDeviceInfo.nSearchTypes,tStart,tEnd))
						{
							nSearchStep =3;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"search remote file fail as startSearchRecFile fail";
							nSearchStep=4;
						}
						pDeviceSearchRecord->Release();
						pDeviceSearchRecord=NULL;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"search remote file fail as it do not support IDeviceSearchRecord interface";
						nSearchStep=4;
					}
					   }
					   break;
				case 3:{
					//搜索成功
					nSearchStep=5;
					   }
					   break;
				case 4:{
					//搜索失败
					//抛出事件
					QVariantMap vItem;
					vItem.insert("parm",QString("%1").arg(1));
					eventCallBack("recFileSearchFail",vItem);
					nSearchStep=5;
					   }
					   break;
				case 5:{
					//释放资源
					if (NULL!=m_pDeviceGroupRemotePlayBack)
					{
						m_pDeviceGroupRemotePlayBack->Release();
						m_pDeviceGroupRemotePlayBack=NULL;
					}else{
						//do nothing
					}
					bSearchStop=true;
					m_bStop=true;
					   }
					   break;
				}
			}
						   }
						   break;
		case RECGROUPSTOP:{
			//停止播放
						  }
						  break;
		case RECGROUPPLAY:{
			//开始播放
						  }
						  break;
		case RECGROUPPAUSE:{
			//暂停播放
						   }
						   break;
		case RECGROUPCONTINUE:{
			//接着播放
							  }
							  break;
		case RECGROUPSPEEDFAST:{
			//快进
							   }
							   break;
		case RECGROUPSEEEDSLOW:{
			//慢放
							   }
							   break;
		case RECGROUPNORMAL:{
			//正常速率播放
							}
							break;
		case RECSETVOLUME:{
			//设置音量
						  }
						  break;
		case RECAUDIOENABLE:{
			//设置音频
							}
							break;
		case RECDEFAULT:{
			if (m_bStop)
			{
				nStep=RECGROUPSTOP;
			}else{
				//do nothing
				sleepEx(10);
			}
						}
						break;
		case RECEND:{

					}
					break;
		}
	}
}

int rPlayBackRunEx::setDeviceHostInfo( const QString &sAddress,unsigned int uiPort,const QString &sEsee )
{
	if (!QThread::isRunning())
	{
		if (!m_tRecDeviceInfo.tAddress.setAddress(sAddress)||uiPort>65535)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as the input param error";
			return 1;
		}else{
			m_tRecDeviceInfo.sAddress=sAddress;
			m_tRecDeviceInfo.uiPort=uiPort;
			m_tRecDeviceInfo.sEsee=sEsee;
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::setDevcieVendor( const QString& sVendor )
{
	if (!QThread::isRunning())
	{
		if (sVendor.isEmpty()||!m_sVendorList.contains(sVendor))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDevcieVendor fail as the input sVendor is empty or undefined";
			return 1;
		}else{
			m_tRecDeviceInfo.sVendor=sVendor;
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDevcieVendor fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::setUserVerifyInfo( const QString &sUserName,const QString &sPassword )
{
	if (!QThread::isRunning())
	{
		m_tRecDeviceInfo.sUserName=sUserName;
		m_tRecDeviceInfo.sPassword=sPassword;
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setUserVerifyInfo fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::addChannelIntoPlayGroup( unsigned int uiWin,int nChannel )
{
	if (!QThread::isRunning())
	{
		tagWinChannelInfo tWinChannelInfo;
		tWinChannelInfo.uiWin=uiWin;
		tWinChannelInfo.nChannel=nChannel;
		m_tWinChannelInfo.insert(uiWin,tWinChannelInfo);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"addChannelIntoPlayGroup fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::startSearchRecFile( int nChannel,int nTypes,const QString &sStartTime,const QString &sEndTime )
{
	if (!QThread::isRunning())
	{
		if (nTypes<0||nTypes>15||sStartTime>=sEndTime)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile as the input params are error";
			return 2;
		}else{
			m_tRecDeviceInfo.sSearchStartTime=sStartTime;
			m_tRecDeviceInfo.sSearchEndTime=sEndTime;
			m_tRecDeviceInfo.nSearchChannel=nChannel;
			m_tRecDeviceInfo.nSearchTypes=nTypes;
			m_qStepCode.clear();
			m_qStepCode.enqueue(RECSEARCHFILE);
			QThread::start();
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::groupPlay( int nTypes,const QString &sStartTime,const QString &sEndTime )
{
	if (!QThread::isRunning())
	{
		if (nTypes<0||nTypes>15||sStartTime>=sEndTime)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as the input param is error";
			return 2;
		}else{
			m_tRecDeviceInfo.nPlayTypes=nTypes;
			m_tRecDeviceInfo.sPlayStartTime=sStartTime;
			m_tRecDeviceInfo.sPlayEndTime=sEndTime;
			m_qStepCode.clear();
			m_qStepCode.enqueue(RECGROUPPLAY);
			QThread::start();
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as work thread had been running,if you want replay please call groupStop()";
		return 1;
	}
	return 0;
}

int rPlayBackRunEx::groupPause()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPPAUSE);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPause fail as the work thread is not running";
		return 1;
	}
}

int rPlayBackRunEx::groupContinue()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPCONTINUE);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupContinue fail as the work thread is not running";
		return 1;
	}
}

int rPlayBackRunEx::groupStop()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPSTOP);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay had been running ,there is no need to call groupStop again";
		return 1;
	}
}

int rPlayBackRunEx::groupSpeedFast()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPSPEEDFAST);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupSpeedFast fail as groupPlay is no in running";
		return 1;
	}
}

int rPlayBackRunEx::groupSpeedSlow()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPSEEEDSLOW);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupSpeedSlow fail as groupPlay is no in running";
		return 1;
	}
}

int rPlayBackRunEx::groupSpeedNormal()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPNORMAL);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupSpeedNormal fail as groupPlay is no in running";
		return 1;
	}
}

int rPlayBackRunEx::setVolume( const unsigned int &uiPersent ,QWidget *pWin)
{
	if (QThread::isRunning())
	{
		m_tRecDeviceInfo.uiPersent=uiPersent;
		m_tRecDeviceInfo.pWin=pWin;
		m_qStepCode.enqueue(RECSETVOLUME);
		return 0;
	}else{
		//do nothing
		return 1;
	}
}

int rPlayBackRunEx::audioEnable( bool bEnable )
{
	if (QThread::isRunning())
	{
		m_tRecDeviceInfo.bAudioEnable=bEnable;
		m_qStepCode.enqueue(RECAUDIOENABLE);
		return 0;
	}else{
		//do nothing
		return 1;
	}
}

QString rPlayBackRunEx::getNowPlayTime()
{
	return "";
}

int rPlayBackRunEx::cbRecRunFoundFile( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunFileSearchFinished( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunFileSearchFail( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunSocketError( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunStateChange( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunCacheState( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

void rPlayBackRunEx::registerEvent( QString sEventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser )
{
	if (!m_tEventNameList.contains(sEventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event :"<<sEventName<<"fail";
		return;
	}else{
		tagRecPlayBackProcInfo tProcInfo;
		tProcInfo.proc=proc;
		tProcInfo.pUser=pUser;
		m_tEventMap.insert(sEventName,tProcInfo);
		return;
	}
}

void rPlayBackRunEx::eventCallBack( QString sEventName,QVariantMap evMap )
{
	if (m_tEventNameList.contains(sEventName))
	{
		tagRecPlayBackProcInfo tProcInfo=m_tEventMap.value(sEventName);
		if (NULL!=tProcInfo.proc)
		{
			tProcInfo.proc(sEventName,evMap,tProcInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<"event is not regist";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support event :"<<sEventName;
	}
}

void rPlayBackRunEx::sleepEx( int nTime )
{
	if (m_nSleepSwitch<100)
	{
		msleep(nTime);
		m_nSleepSwitch++;
	}else{
		QEventLoop tEventLoop;
		QTimer::singleShot(2,&tEventLoop,SLOT(quit()));
		tEventLoop.exec();
		m_nSleepSwitch=0;
	}
}

bool rPlayBackRunEx::getDeviceObject()
{
	bool bFlag=false;
	QString sAppPath=QCoreApplication::applicationDirPath();
	QFile *pFile=new QFile(sAppPath + "/pcom_config.xml");
	pFile->open(QIODevice::ReadOnly);
	QDomDocument tConfFile;
	tConfFile.setContent(pFile);

	QDomNode tClsidNode=tConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList tItemList=tClsidNode.childNodes();
	for (int n=0;n<tItemList.count();n++)
	{
		QDomNode tItem=tItemList.at(n);
		QString sItemName=tItem.toElement().attribute("vendor");
		if (sItemName==m_tRecDeviceInfo.sVendor)
		{
			CLSID cPlayBackTypeClsid=pcomString2GUID(tItem.toElement().attribute("clsid"));
			pcomCreateInstance(cPlayBackTypeClsid,NULL,IID_IDeviceGroupRemotePlayback,(void **)m_pDeviceGroupRemotePlayBack);
			if (NULL!=m_pDeviceGroupRemotePlayBack)
			{
				bFlag=true;
				break;
			}
		}
	}
	if (NULL!=pFile)
	{
		delete pFile;
		pFile=NULL;
	}else{
		//do nothing
	}
	return bFlag;
}

int cbXRecRunFoundFile( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunFoundFile(evName,evMap,pUser);
}

int cbXRecRunFileSearchFinished( QString evName,QVariantMap evMap,void*pUser )
{

	return ((rPlayBackRunEx*)pUser)->cbRecRunFileSearchFinished(evName,evMap,pUser);
}

int cbXRecRunFileSearchFail( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunFileSearchFail(evName,evMap,pUser);
}

int cbXRecRunSocketError( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunSocketError(evName,evMap,pUser);
}

int cbXRecRunStateChange( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunStateChange(evName,evMap,pUser);
}

int cbXRecRunCacheState( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunCacheState(evName,evMap,pUser);
}
