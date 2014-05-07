#include <QtGui/QResizeEvent>
#include <qwfw_tools.h>
#include <QtCore/QtCore>
#include <QtXml/QtXml>
#include <qtconcurrentrun.h>
#include <QtCore/QFuture>
#include <QElapsedTimer>
#include <libpcom.h>
#include <guid.h>
#include <IDeviceSearchRecord.h>
#include <IDeviceConnection.h>
#include "rPlaybackWnd.h"
#include "RemotePlaybackPlugin_global.h"

RPlaybackWnd::RPlaybackWnd(QWidget *parent)
: QWidget(parent),
QWebPluginFWBase(this),
m_uiPort(80),
m_nCurrentWnd(0),
m_uiRecFileSearched(0),
m_DivMode(NULL),
m_GroupPlayback(NULL),
bIsInitFlags(false),
bIsCaseInitFlags(false),
_curConnectState(STATUS_DISCONNECTED),
_curConnectType(TYPE_NULL),
bIsOpenAudio(false),
m_uiPersent(50),
m_CurStatus(STATUS_STOP)
// m_DeviceClient(NULL)
{
	for (int i = 0; i < ARRAY_SIZE(m_PlaybackWnd); ++i)
	{
		m_PlaybackWnd[i].setParent(this);
		connect(&m_PlaybackWnd[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_PlaybackWnd[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));

		m_PlaybackWndList.insert(m_PlaybackWndList.size(),&m_PlaybackWnd[i]);
	}
	m_PlaybackWnd[0].SetFoucs(true);
	pcomCreateInstance(CLSID_DivMode2_2,NULL,IID_IWindowDivMode,(void **)&m_DivMode);
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->setParentWindow(this);
  		m_DivMode->setSubWindows(m_PlaybackWndList,ARRAY_SIZE(m_PlaybackWnd));
  		m_DivMode->flush();
 	}
	m_RemotePlaybackObject.SetrPlaybackWnd(this);
	m_rplaybackrun.cbRegisterEvent("foundFile",cbFoundFile,this);
	m_rplaybackrun.cbRegisterEvent("CurrentStatus",cbStateChange,this);
	m_rplaybackrun.cbRegisterEvent("recFileSearchFinished",cbRecFileSearchFinished,this);
	m_rplaybackrun.cbRegisterEvent("bufferStatus",cbCacheState,this);
	bool flag=false;
	connect(this,SIGNAL(FoundFileToUiS(QVariantMap)),this,SLOT(FoundFileToUislot(QVariantMap)));
 	connect(this,SIGNAL(RecFileSearchFinishedToUiS(QVariantMap)),this,SLOT(RecFileSearchFinishedToUislot(QVariantMap)));
 	connect(this,SIGNAL(SocketErrorToUiS(QVariantMap)),this,SLOT(SocketErrorToUislot(QVariantMap)));
 	connect(this,SIGNAL(CacheStateToUiS(QVariantMap)),this,SLOT(CacheStateToUislot(QVariantMap)));
}

RPlaybackWnd::~RPlaybackWnd()
{
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->Release();
 		m_DivMode = NULL;
 	}

    if (m_GroupPlayback != NULL)
    {
        m_GroupPlayback->Release();
        m_GroupPlayback = NULL;
    }
}

void  RPlaybackWnd::resizeEvent( QResizeEvent * ev)
{
	if (NULL != m_DivMode)
	{
		m_DivMode->parentWindowResize(ev);
	}
}

int   RPlaybackWnd::setDeviceHostInfo(const QString & sAddress,unsigned int uiPort,const QString &eseeID)
{
	qDebug()<<"RPlaybackWnd setDeviceHostInfo"<<sAddress<<uiPort<<eseeID;
	QVariantMap item;
	m_rplaybackrun.setDeviceHostInfo(sAddress,uiPort,eseeID);
    if (!m_HostAddress.setAddress(sAddress) || uiPort > 65535)
    {
        return 1;
    }
	qDebug()<<m_HostAddress.toString();
	m_sHostAddress.clear();
	m_sHostAddress=m_HostAddress.toString();
    m_uiPort   = uiPort;
    m_sEseeId  = eseeID;
    return 0;
}

int   RPlaybackWnd::setDeviceVendor(const QString & vendor)
{
	qDebug()<<"RPlaybackWnd setDeviceVendor"<<vendor;
	m_rplaybackrun.setDeviceVendor(vendor);
    if (vendor.isEmpty())
    {
        return 1;
    }
    m_sVendor = vendor;
    return 0;
}

int RPlaybackWnd::AddChannelIntoPlayGroup( uint uiWndId,int uiChannelId )
{
	qDebug()<<"RPlaybackWnd AddChannelIntoPlayGroup"<<uiWndId<<uiChannelId;
	GetDeviceInfo(uiChannelId);
	if (uiWndId >= ARRAY_SIZE(m_PlaybackWnd) || m_DevCliSetInfo.m_uiChannelId > 32)
	{
		return -1;
	}
	int nRet = 2;
	m_chlID=uiChannelId;
	if (!bIsCaseInitFlags)
	{
		QString sAppPath = QCoreApplication::applicationDirPath();
		QFile * file = new QFile(sAppPath + "/pcom_config.xml");
		file->open(QIODevice::ReadOnly);
		QDomDocument ConfFile;
		ConfFile.setContent(file);

		QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
		QDomNodeList itemList = clsidNode.childNodes();

		for (int n = 0; n < itemList.count(); n++)
		{
			QDomNode item = itemList.at(n);
			QString sItemName = item.toElement().attribute("vendor");

			if (sItemName == m_sVendor)
			{
				if (NULL != m_GroupPlayback)
				{
					m_GroupPlayback->Release();
					m_GroupPlayback = NULL;
				}
				CLSID playbackTypeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
				pcomCreateInstance(playbackTypeClsid,NULL,IID_IDeviceGroupRemotePlayback,(void **)&m_GroupPlayback);
				for (int i=0;i< ARRAY_SIZE(m_PlaybackWnd); ++i)
				{
					m_PlaybackWnd[i].SetLpClient(m_GroupPlayback);
				}

				m_RemotePlaybackObject.SetIDeviceGroupRemotePlaybackParm(m_GroupPlayback);
				bIsCaseInitFlags = true;
				break;
			}
		}
		file->close();
		delete file;
	}
	if (NULL != m_GroupPlayback)
	{
		nRet = m_GroupPlayback->AddChannelIntoPlayGroup(m_DevCliSetInfo.m_uiChannelId, &m_PlaybackWnd[uiWndId]);
		_widList<<uiWndId;
		_widInfo.insert(uiWndId,m_DevCliSetInfo.m_uiChannelIdInDataBase);
	}
	else
	{
		bIsCaseInitFlags = false;
	}
	return nRet;
}

void   RPlaybackWnd::setUserVerifyInfo(const QString & sUsername,const QString & sPassword)
{
	qDebug()<<"RPlaybackWnd setUserVerifyInfo"<<sUsername<<sPassword;
    m_rplaybackrun.setUserVerifyInfo(sUsername,sPassword);
	m_sUserName = sUsername;
    m_sUserPwd  = sPassword;
}

int   RPlaybackWnd::startSearchRecFile(int nChannel,int nTypes,const QString & startTime,const QString & endTime)
{
	qDebug()<<"RPlaybackWnd:"<<"nChannel"<<nChannel<<"nTypes"<<nTypes<<startTime<<endTime;
	m_rplaybackrun.startSearchRecFile(nChannel,nTypes,startTime,endTime);
	int nRet=1;
	if (false==bIsInitFlags)
	{
		if (1==cbInit())
		{
			goto finishSearch;
		}
	}
	if (NULL==m_GroupPlayback)
	{
		goto finishSearch;
	}
	nRet=m_RemotePlaybackObject.SetParm(m_DevCliSetInfo.m_sUsername,m_DevCliSetInfo.m_sPassword,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_sEseeId);
	if (1==nRet)
	{
		goto finishSearch;
	}
	_curConnectType=TYPE_SEARCH;
	/*nRet=m_RemotePlaybackObject.startSearchRecFile(nChannel,nTypes,startTime,endTime);*/
	return nRet;
finishSearch:
	{
	QVariantMap item;
	item.insert("total",0);
	RecFileSearchFinished(item);
	}
	return nRet;
}


 QString RPlaybackWnd::GetNowPlayedTime()
 {
	 if (NULL == m_GroupPlayback||m_CurStatus==STATUS_STOP)
	 {
		 return "-1";
	 }
     QDateTime playedTime = m_GroupPlayback->GroupGetPlayedTime();

	 //QString playedTimeTest;
	 //playedTimeTest=playedTime.toString("yyyy-MM-dd hh:mm:ss");
	 //qDebug()<<playedTimeTest;

	 QDateTime pCurdate;
	 pCurdate.setDate(QDate::currentDate());
	 QString CurrentTime;
	 CurrentTime=QString("%1").arg(playedTime.toTime_t()-pCurdate.toTime_t());
	 return CurrentTime;
 }

int   RPlaybackWnd::GroupPlay(int nTypes,const QString & startTime,const QString & endTime)
{
	qDebug()<<"RPlaybackWnd GroupPlay"<<"nTypes"<<nTypes<<startTime<<endTime;
	int nRet=1;
	if (false==bIsInitFlags)
	{
		if (1==cbInit())
		{
			return nRet;
		}
	}
	/*nRet=m_RemotePlaybackObject.SetParm(m_sUserName,m_sUserPwd,m_uiPort,m_sHostAddress,m_sEseeId);*/
	nRet=m_RemotePlaybackObject.SetParm(m_DevCliSetInfo.m_sUsername,m_DevCliSetInfo.m_sPassword,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_sEseeId);
	if (1==nRet)
	{
		return nRet;
	}
	_curConnectType=TYPE_STREAM;
	nRet=m_RemotePlaybackObject.GroupPlay(nTypes,startTime,endTime);
	if (_widList.isEmpty()==false)
	{
		_mutexWidList.lock();
		QList<int>::Iterator it;
		for(it=_widList.begin();it!=_widList.end();it++){
			m_PlaybackWnd[*it].saveCacheImage();
		}
		_mutexWidList.unlock();
	}
	m_CurStatus=STATUS_PLAY;
	/*SetVolume(0xAECBCA);*/
	AudioEnabled(bIsOpenAudio);
	SetVolume(m_uiPersent);
	return nRet;
}

int   RPlaybackWnd::GroupPause()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupPause();
    } 
	m_CurStatus=STATUS_PAUSE;
    return nRet;
}
int   RPlaybackWnd::GroupContinue()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupContinue();
    } 
	m_CurStatus=STATUS_CONTINUE;
    return nRet;
}
int   RPlaybackWnd::GroupStop()
{
	qDebug()<<"RPlaybackWnd:GroupStop";
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupStop();
		_widList.clear();
    } 
	m_CurStatus=STATUS_STOP;
    return nRet;
}
int  RPlaybackWnd::AudioEnabled(bool bEnable)
{
    int bRet = m_PlaybackWnd[0].AudioEnabled(bEnable);
	bIsOpenAudio=bEnable;
	if (NULL != m_GroupPlayback)
	{
		m_GroupPlayback->GroupSetVolume(0xAECBCA, &m_PlaybackWnd[m_nCurrentWnd]);
	}
    return bRet;
}
int   RPlaybackWnd::SetVolume(const unsigned int &uiPersent)
{
	int nRet = -1;
	m_uiPersent=uiPersent;
	if (NULL != m_GroupPlayback)
	{
		nRet = m_GroupPlayback->GroupSetVolume(uiPersent, &m_PlaybackWnd[m_nCurrentWnd]);
	}
	return nRet;
}
int   RPlaybackWnd::GroupSpeedFast() 
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupSpeedFast();
    } 
	m_CurStatus=STATUS_FAST;
    return nRet;
}
int   RPlaybackWnd::GroupSpeedSlow()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupSpeedSlow();
    } 
	m_CurStatus=STATUS_SLOW;
    return nRet;
}
int   RPlaybackWnd::GroupSpeedNormal()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupSpeedNormal();
    } 
	m_CurStatus=STATUS_NORMAL;
    return nRet;
}

int   RPlaybackWnd::GetCurrentWnd()
{
    return m_nCurrentWnd;
}
      
void  RPlaybackWnd::OnSubWindowDblClick( QWidget * wind,QMouseEvent * ev)
{
    if (NULL==m_DivMode)
    {
        return ;
    }
    m_DivMode->subWindowDblClick(wind,ev);
}
      
void  RPlaybackWnd::SetCurrentWind(QWidget *wind)
{
    int j;
    for (j = 0; j < ARRAY_SIZE(m_PlaybackWnd); ++j)
    {
        if (&m_PlaybackWnd[j]==wind)
        {
			m_PlaybackWnd[j].SetFoucs(true);
            m_nCurrentWnd=j;
		}else{
			m_PlaybackWnd[j].SetFoucs(false);
		}		
    }   
}
      
void  RPlaybackWnd::CurrentStateChangePlugin(int statevalue)
{
    DEF_EVENT_PARAM(arg);
    EP_ADD_PARAM(arg,"CurrentState",statevalue);
    EventProcCall("CurrentStateChange",arg);
}

int  RPlaybackWnd::GetRecFileNum(uint uiNum)
{
    m_uiRecFileSearched = uiNum;
    return uiNum;
}
int  RPlaybackWnd::cbInit()
 {
     if (NULL == m_GroupPlayback)
     {
         return 1;
     }
     QString evName = "foundFile";
     IEventRegister *pRegist = NULL;
     m_GroupPlayback->QueryInterface(IID_IEventRegister,(void**)&pRegist);
     if (NULL == pRegist)
     {
         return 1;
     }
     pRegist->registerEvent(evName,cbFoundFile,this);
     evName.clear();
     evName.append("CurrentStatus");
     pRegist->registerEvent(evName,cbStateChange,this);
     evName.clear();
     evName.append("recFileSearchFinished");
     pRegist->registerEvent(evName,cbRecFileSearchFinished,this);
     pRegist->Release();
	 evName.clear();
	 evName.append("bufferStatus");
	 pRegist->registerEvent(evName,cbCacheState,this);
     pRegist=NULL;

     bIsInitFlags=true;
     return 0;
 }

void RPlaybackWnd::FoundFile( QVariantMap evMap )
{
	QVariantMap::const_iterator it;
	QString fileinfo;
	fileinfo.append("{");
	for(it=evMap.begin();it!=evMap.end();it++){
		/*fileinfo.append(it.key()).append(":").append(it.value().toString()).append(";");*/
		fileinfo.append("\\").append("\"").append(it.key()).append("\\").append("\"").append(":").append("\\").append("\"").append(it.value().toString()).append("\\").append("\"").append(",");
	}
	fileinfo.append("}");
	fileinfo.replace(",}","}");
	fileMap.insert(fileKey,fileinfo);
	int ifilekey;
	ifilekey=fileKey.toInt();
	fileKey=QString::number(ifilekey+1);
	if (ifilekey==fileTotal-1)
	{
		emit FoundFileToUiS(fileMap);
	}
}

void RPlaybackWnd::RecFileSearchFinished( QVariantMap evMap )
{
	qDebug()<<evMap;
	fileTotal=evMap.value("total").toInt();
	fileKey="0";
	fileMap.clear();
	emit RecFileSearchFinishedToUiS(evMap);
}

void RPlaybackWnd::SocketError( QVariantMap evMap )
{

}

void RPlaybackWnd::StateChange( QVariantMap evMap )
{
	emit StateChangeToUiS(evMap);
}
void RPlaybackWnd::CacheState( QVariantMap evMap )
{
	emit CacheStateToUiS(evMap);
}

void RPlaybackWnd::hideEvent( QHideEvent * )
{
	m_PlaybackWnd[0].AudioEnabled(false);
	GroupPause();
}

void RPlaybackWnd::showEvent( QShowEvent * )
{
	m_PlaybackWnd[0].AudioEnabled(bIsOpenAudio);
	if (NULL != m_GroupPlayback)
	{
		 m_GroupPlayback->GroupSetVolume(0xAECBCA, &m_PlaybackWnd[m_nCurrentWnd]);
	}
	if (ChlIsExit(m_chlID))
	{
		GroupContinue();
	}else{
		GroupStop();
	}
	
}

QVariantMap RPlaybackWnd::ScreenShot()
{
	return m_PlaybackWnd[m_nCurrentWnd].ScreenShot();
}

int RPlaybackWnd::GetDeviceInfo( int chlId )
{
	if (chlId==m_DevCliSetInfo.m_uiChannelIdInDataBase)
	{
		return 1;
	}
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&pChannelManager);
	if (pChannelManager!=NULL)
	{
		QVariantMap channelInfo=pChannelManager->GetChannelInfo(chlId);
		m_DevCliSetInfo.m_uiStreamId=channelInfo.value("stream").toInt();
		m_DevCliSetInfo.m_uiChannelId=channelInfo.value("number").toInt();
		m_DevCliSetInfo.m_sCameraname=channelInfo.value("name").toString();
		int dev_id=channelInfo.value("dev_id").toInt();
		pChannelManager->Release();
		IDeviceManager *pDeviceManager=NULL;
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void **)&pDeviceManager);
		if (pDeviceManager!=NULL)
		{
			QVariantMap deviceInfo=pDeviceManager->GetDeviceInfo(dev_id);
			m_DevCliSetInfo.m_sVendor=deviceInfo.value("vendor").toString();
			m_DevCliSetInfo.m_sPassword=deviceInfo.value("password").toString();
			m_DevCliSetInfo.m_sUsername=deviceInfo.value("username").toString();
			m_DevCliSetInfo.m_sEseeId=deviceInfo.value("eseeid").toString();
			m_DevCliSetInfo.m_sAddress=deviceInfo.value("address").toString();
			m_DevCliSetInfo.m_uiPort=deviceInfo.value("port").toInt();
			m_DevCliSetInfo.m_uiChannelIdInDataBase=chlId;
			pDeviceManager->Release();
			return 0;
		}
	}
	return 1;
}

int RPlaybackWnd::GetWndInfo( int uiWndId )
{
	return _widInfo[uiWndId];
}

bool RPlaybackWnd::ChlIsExit( int chlId )
{
	bool flags=false;
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&pChannelManager);
	if (pChannelManager!=NULL)
	{
		QVariantMap channelInfo=pChannelManager->GetChannelInfo(chlId);
		if (channelInfo.value("dev_id").toInt()==-1)
		{
			flags=false;
		}else{
			flags=true;
		}
		pChannelManager->Release();
	}
	return flags;
}

void RPlaybackWnd::FoundFileToUislot( QVariantMap evMap )
{
	EventProcCall("RecFileInfo",evMap);
}

void RPlaybackWnd::RecFileSearchFinishedToUislot( QVariantMap evMap )
{
	qDebug()<<evMap;
	EventProcCall("recFileSearchFinished",evMap);
}

void RPlaybackWnd::SocketErrorToUislot( QVariantMap evMap )
{

}

void RPlaybackWnd::StateChangeToUislot( QVariantMap evMap )
{
	_curConnectState=(__enConnectStatus)evMap.value("CurrentStatus").toInt();
	if (_curConnectType==TYPE_STREAM)
	{
		qDebug()<<evMap;
		QList<int>::Iterator it;
		for(it=_widList.begin();it!=_widList.end();it++){
			m_PlaybackWnd[*it].SetCurConnectState((RSubView::__enConnectStatus)_curConnectState);
		}
	}
	if (_curConnectState==STATUS_DISCONNECTED)
	{
		_mutexWidList.lock();
		_widList.clear();
		_mutexWidList.unlock();
	}
}

void RPlaybackWnd::CacheStateToUislot( QVariantMap evMap )
{
	QList<int>::iterator it;
	for(it=_widList.begin();it!=_widList.end();it++){
		m_PlaybackWnd[*it].CacheState(evMap);
	}
}

 int cbFoundFile(QString evName,QVariantMap evMap,void*pUser)
 {
     int nRet = 1;
     if (evName == "foundFile")
     {
		 ((RPlaybackWnd*)pUser)->FoundFile(evMap);
     }
     return nRet;
 }

 int cbRecFileSearchFinished(QString evName,QVariantMap evMap,void*pUser)
 {
     int nRet = 1;
     if (evName == "recFileSearchFinished")
     {
		 ((RPlaybackWnd*)pUser)->RecFileSearchFinished(evMap);
     }
     return nRet;
 }

 int cbStateChange(QString evName,QVariantMap evMap,void*pUser)
 {
	 if (evName=="CurrentStatus")
	 {
		 ((RPlaybackWnd*)pUser)->StateChange(evMap);
	 }
	 return 1;
 }
 












 int cbCacheState(QString evName,QVariantMap evMap,void*pUser)
 {
		 if (evName=="bufferStatus")
		 {
			 ((RPlaybackWnd*)pUser)->CacheState(evMap);
		 }
		 return 1;
 }
