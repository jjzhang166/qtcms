#include "rPlayBackRun.h"

rPlayBackRun::rPlayBackRun(void)
{
	m_timer=new QTimer(this);
	connect(m_timer,SIGNAL(timeout()),this,SLOT(__ResetForbinFreOpera()));
	m_timer->start(1000);
	QThread::start();
	m_FuncIndex.insert("__startSearchRecFile",1);
	m_bStop=false;
}


rPlayBackRun::~rPlayBackRun(void)
{
	m_brunning=false;
	while(QThread::isRunning()){
		msleep(10);
	}
	delete m_timer;
}

void rPlayBackRun::run()
{
	while(m_brunning){
		if (m_FunctionMap.size()!=0)
		{
			m_FunctionMapMutex.lock();
			m_FuncInfo.clear();
			m_FuncInfo=m_FunctionMap.dequeue();
			m_FunctionMapMutex.unlock();
			__SwitchFunction();
		}
		msleep(10);
	}
}

int rPlayBackRun::setDeviceHostInfo( const QString & sAddress,unsigned int uiPort,const QString &eseeID )
{
	m_RunDevInfo.m_sAddress=sAddress;
	m_RunDevInfo.m_uiPort=uiPort;
	m_RunDevInfo.m_sEseeId=eseeID;
	return 0;
}

int rPlayBackRun::setDeviceVendor( const QString & vendor )
{
	m_RunDevInfo.m_sVendor=vendor;
	return 0;
}

int rPlayBackRun::AddChannelIntoPlayGroup( uint uiWndId,int uiChannelId )
{
	return 0;
}

int rPlayBackRun::GetWndInfo( int uiWndId )
{
	return 0;
}

void rPlayBackRun::setUserVerifyInfo( const QString & sUsername,const QString & sPassword )
{
	m_RunDevInfo.m_sUsername=sUsername;
	m_RunDevInfo.m_sPassword=sPassword;
}

int rPlayBackRun::startSearchRecFile( int nChannel,int nTypes,const QString & startTime,const QString & endTime )
{
	QVariantMap item;
	item.insert("cbfun","__startSearchRecFile");
	item.insert("nChannel",nChannel);
	item.insert("nTypes",nTypes);
	item.insert("startTime",startTime);
	item.insert("endTime",endTime);
	item.insert("sUsername",m_RunDevInfo.m_sUsername);
	item.insert("sPassword",m_RunDevInfo.m_sPassword);
	item.insert("vendor",m_RunDevInfo.m_sVendor);
	item.insert("sAddress",m_RunDevInfo.m_sAddress);
	item.insert("uiPort",m_RunDevInfo.m_uiPort);
	item.insert("eseeID",m_RunDevInfo.m_sEseeId);
	__InsertFunction(item);
	return 0;
}

QString rPlayBackRun::GetNowPlayedTime()
{
	return "0";
}

int rPlayBackRun::GroupPlay( int nTypes,const QString & start,const QString & end )
{
	return 0;
}

int rPlayBackRun::GroupPause()
{
	return 0;
}

int rPlayBackRun::GroupContinue()
{
	return 0;
}

int rPlayBackRun::GroupStop()
{
	return 0;
}

int rPlayBackRun::AudioEnabled( bool bEnable )
{
	return 0;
}

int rPlayBackRun::SetVolume( const unsigned int &uiPersent )
{
	return 0;
}

int rPlayBackRun::GroupSpeedFast()
{
	return 0;
}

int rPlayBackRun::GroupSpeedSlow()
{
	return 0;
}

int rPlayBackRun::GroupSpeedNormal()
{
	return 0;
}



int rPlayBackRun::__AddChannelIntoPlayGroup( uint uiWndId,int uiChannelId )
{
	return 0;
}

int rPlayBackRun::__GetWndInfo( int uiWndId )
{
	return 0;
}

bool rPlayBackRun::__startSearchRecFile()
{
	IDeviceGroupRemotePlayback * m_GroupPlaySearch=NULL;
	__BuildDev((void**)&m_GroupPlaySearch);
	int nret=1;
	bool flag=false;
	if (NULL!=m_GroupPlaySearch)
	{
		if (__InitSearchCb(m_GroupPlaySearch))
		{
			int nChannel=m_FuncInfo.value("nChannel").toInt();
			int nTypes=m_FuncInfo.value("nTypes").toInt();
			QString startTime=m_FuncInfo.value("startTime").toString();
			QString endTime=m_FuncInfo.value("endTime").toString();
			QString sUsername=m_FuncInfo.value("sUsername").toString();
			QString sPassword=m_FuncInfo.value("sPassword").toString();
			QString sAddress=m_FuncInfo.value("sAddress").toString();
			int uiPort=m_FuncInfo.value("uiPort").toInt();
			QString eseeID=m_FuncInfo.value("eseeID").toString();
			if (startTime.isEmpty()==false&&endTime.isEmpty()==false)
			{
				IDeviceClient *m_nIDeviceClient=NULL;
				m_GroupPlaySearch->QueryInterface(IID_IDeviceClient,(void**)&m_nIDeviceClient);
				if (m_nIDeviceClient!=NULL)
				{
					m_nIDeviceClient->checkUser(sUsername,sPassword);
					m_nIDeviceClient->setDeviceHost(sAddress);
					m_nIDeviceClient->setDeviceId(eseeID);
					m_nIDeviceClient->setDevicePorts(uiPort);
					/*m_nIDeviceClient->connectToDevice();*/
					IDeviceSearchRecord *m_DeviceSearchRecord=NULL;
					m_GroupPlaySearch->QueryInterface(IID_IDeviceSearchRecord,(void**)&m_DeviceSearchRecord);
					if (NULL!=m_DeviceSearchRecord)
					{
						QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
						QDateTime end   = QDateTime::fromString(endTime,   "yyyy-MM-dd hh:mm:ss");
						nret =m_DeviceSearchRecord->startSearchRecFile(nChannel,nTypes,start,end);
						if (0==nret)
						{
							flag=true;
						}
						m_DeviceSearchRecord->Release();
						m_DeviceSearchRecord=NULL;
					}
					m_nIDeviceClient->Release();
					m_nIDeviceClient=NULL;
				}
			}
			m_GroupPlaySearch->Release();
			m_GroupPlaySearch=NULL;
		}
	}
	return flag;
}

QString rPlayBackRun::__GetNowPlayedTime()
{
	return "0";
}

int rPlayBackRun::__GroupPlay( int nTypes,const QString & start,const QString & end )
{
	return 0;
}

int rPlayBackRun::__GroupPause()
{
	return 0;
}

int rPlayBackRun::__GroupContinue()
{
	return 0;
}

int rPlayBackRun::__GroupStop()
{
	return 0;
}

int rPlayBackRun::__AudioEnabled( bool bEnable )
{
	return 0;
}

int rPlayBackRun::__SetVolume( const unsigned int &uiPersent )
{
	return 0;
}

int rPlayBackRun::__GroupSpeedFast()
{
	return 0;
}

int rPlayBackRun::__GroupSpeedSlow()
{
	return 0;
}

int rPlayBackRun::__GroupSpeedNormal()
{
	return 0;
}

int rPlayBackRun::__InsertFunction( QVariantMap evMap )
{
	for (int i=0;i<m_FunctionMap.size();++i)
	{
		QVariantMap item=m_FunctionMap.at(i);
		if (item==evMap&&m_ForbinFreOpera==false)
		{
			return 0;
		}
	}
	m_ForbinFreOpera=false;
	m_FunctionMapMutex.lock();
	m_FunctionMap.enqueue(evMap);
	m_FunctionMapMutex.unlock();
	return 0;
}

void rPlayBackRun::__ResetForbinFreOpera()
{
	m_ForbinFreOpera=true;
	m_bStop=true;
}

bool rPlayBackRun::__BuildDev(void **ppv)
{
	bool flag=false;
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	for (int n=0;n<itemList.count();n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("vendor");
		if (sItemName==m_RunDevInfo.m_sVendor)
		{
			CLSID playbackTypeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(playbackTypeClsid,NULL,IID_IDeviceGroupRemotePlayback,(void **)ppv);
			if (NULL!=ppv)
			{
				flag=true;
				break;
			}
		}
	}
	if (NULL!=file)
	{
		delete file;
		file=NULL;
	}
	return flag;
}

bool rPlayBackRun::__InitSearchCb(IDeviceGroupRemotePlayback *pSearch)
{
	if (pSearch!=NULL)
	{
		QString evName = "foundFile";
		IEventRegister *pRegist = NULL;
		pSearch->QueryInterface(IID_IEventRegister,(void**)&pRegist);
		if (NULL!=pRegist)
		{
			QMultiMap<QString,   EventCBInfo>::Iterator i=m_mEventCBMap.constBegin();
			while(i!=m_mEventCBMap.constEnd()){
				EventCBInfo item=i.value();
				pRegist->registerEvent(i.key(),item.evCBName,item.pUser);
				++i;
			}
			pRegist->Release();
			pRegist=NULL;
			return true;
		}
	}
	return false;
}

void rPlayBackRun::__SwitchFunction()
{
	QString cbFuncName=m_FuncInfo.value("cbfun").toString();
	int index=m_FuncIndex.value(cbFuncName).toInt();
	switch(index){
	case 1:__startSearchRecFile();
		break;
	case 2:
		__GroupPause();
		break;
	}
}

void rPlayBackRun::cbRegisterEvent( QString eventName,runEventCallBack eventCB,void *pUser )
{
	EventCBInfo procInfo;
	procInfo.evCBName = eventCB;
	procInfo.pUser = pUser;
	m_mEventCBMap.insert(eventName, procInfo);
}


