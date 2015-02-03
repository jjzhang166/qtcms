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
#include "ILocalSetting.h"
#include <QRegExp>

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
m_lastStatus(STATUS_STOP),
m_CurStatus(STATUS_STOP),
bIsHide(false)
// m_DeviceClient(NULL)
{
	for (int i = 0; i < ARRAY_SIZE(m_PlaybackWnd); ++i)
	{
		m_PlaybackWnd[i].setParent(this);
		m_PlaybackWnd[i].setCbpfn(cbDigitalZoom, &m_rplaybackrun);
		connect(&m_PlaybackWnd[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_PlaybackWnd[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));
		connect(&m_PlaybackWnd[i],SIGNAL(sigValidateFail(QVariantMap)), this, SLOT(slValidateFail(QVariantMap)));

		m_PlaybackWndList.insert(m_PlaybackWndList.size(),&m_PlaybackWnd[i]);
		m_rplaybackrun.setPlaybackWnd(&m_PlaybackWnd[i]);
	}
	m_PlaybackWnd[0].SetFoucs(true);
	pcomCreateInstance(CLSID_DivMode2_2,NULL,IID_IWindowDivMode,(void **)&m_DivMode);
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->setParentWindow(this);
  		m_DivMode->setSubWindows(m_PlaybackWndList,ARRAY_SIZE(m_PlaybackWnd));
  		m_DivMode->flush();
 	}
	bool flag=false;
	connect(&m_rplaybackrun,SIGNAL(FoundFileToUiS(QVariantMap)),this,SLOT(FoundFileToUislot(QVariantMap)));
 	connect(&m_rplaybackrun,SIGNAL(RecFileSearchFinishedToUiS(QVariantMap)),this,SLOT(RecFileSearchFinishedToUislot(QVariantMap)));
	connect(&m_rplaybackrun,SIGNAL(FileSearchFailToUiS(QVariantMap)),this,SLOT(FileSearchFailUislot(QVariantMap)));
	connect(&m_rplaybackrun,SIGNAL(StateChangeToUiS(QVariantMap)), this, SLOT(StateChangeToUislot(QVariantMap)));
	connect(&m_rplaybackrun,SIGNAL(FileSearchStartToUiS(QVariantMap)),this,SLOT(FileSearchStartUislot(QVariantMap)));
	connect(&m_rplaybackrun,SIGNAL(sgScreenShot(QVariantMap)),this,SLOT(slScreenShot(QVariantMap)));
	QApplication::installTranslator(&m_translator);
}

RPlaybackWnd::~RPlaybackWnd()
{
	m_PlaybackWnd[0].destroySusWnd();
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
// 	QVariantMap item;
	QRegExp rx("^([1-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])(\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])){3}$");
	if (sAddress.contains(rx) && uiPort > 65535){
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as the input param error";
		return 1;
	}

	m_rplaybackrun.setDeviceHostInfo(sAddress,uiPort,eseeID);
//     if (!m_HostAddress.setAddress(sAddress) || uiPort > 65535)
//     {
//         return 1;
//     }
// 	m_sHostAddress.clear();
// 	m_sHostAddress=m_HostAddress.toString();
//     m_uiPort   = uiPort;
//     m_sEseeId  = eseeID;
    return 0;
}

int RPlaybackWnd::setDeviceVendor(const QString & vendor)
{
    if (vendor.isEmpty())
    {
        return 1;
    }
	m_rplaybackrun.setDeviceVendor(vendor);
//     m_sVendor = vendor;
    return 0;
}

int RPlaybackWnd::AddChannelIntoPlayGroup( uint uiWndId,int uiChannelId )
{
// 	if (uiWndId >= ARRAY_SIZE(m_PlaybackWnd) || m_DevCliSetInfo.m_uiChannelId > 32)
// 	{
// 		return -1;
// 	}
	m_chlID = uiChannelId;
	return m_rplaybackrun.AddChannelIntoPlayGroup(uiWndId, uiChannelId);

// 	GetDeviceInfo(uiChannelId);
// 	if (uiWndId >= ARRAY_SIZE(m_PlaybackWnd) || m_DevCliSetInfo.m_uiChannelId > 32)
// 	{
// 		return -1;
// 	}
// 	int nRet = 2;
// 	m_chlID=uiChannelId;
// 	if (!bIsCaseInitFlags)
// 	{
// 		QString sAppPath = QCoreApplication::applicationDirPath();
// 		QFile * file = new QFile(sAppPath + "/pcom_config.xml");
// 		file->open(QIODevice::ReadOnly);
// 		QDomDocument ConfFile;
// 		ConfFile.setContent(file);
// 
// 		QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
// 		QDomNodeList itemList = clsidNode.childNodes();
// 
// 		for (int n = 0; n < itemList.count(); n++)
// 		{
// 			QDomNode item = itemList.at(n);
// 			QString sItemName = item.toElement().attribute("vendor");
// 
// 			if (sItemName == m_sVendor)
// 			{
// 				if (NULL != m_GroupPlayback)
// 				{
// 					m_GroupPlayback->Release();
// 					m_GroupPlayback = NULL;
// 				}
// 				CLSID playbackTypeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
// 				pcomCreateInstance(playbackTypeClsid,NULL,IID_IDeviceGroupRemotePlayback,(void **)&m_GroupPlayback);
// 				for (int i=0;i< ARRAY_SIZE(m_PlaybackWnd); ++i)
// 				{
// 					m_PlaybackWnd[i].SetLpClient(m_GroupPlayback);
// 				}
// 
// 				m_RemotePlaybackObject.SetIDeviceGroupRemotePlaybackParm(m_GroupPlayback);
// 				bIsCaseInitFlags = true;
// 				break;
// 			}
// 		}
// 		file->close();
// 		delete file;
// 	}
// 	if (NULL != m_GroupPlayback)
// 	{
// 		nRet = m_GroupPlayback->AddChannelIntoPlayGroup(m_DevCliSetInfo.m_uiChannelId, &m_PlaybackWnd[uiWndId]);
// 		_widList<<uiWndId;
// 		_widInfo.insert(uiWndId,m_DevCliSetInfo.m_uiChannelIdInDataBase);
// 	}
// 	else
// 	{
// 		bIsCaseInitFlags = false;
// 	}
// 	return nRet;
}

void RPlaybackWnd::setUserVerifyInfo(const QString & sUsername,const QString & sPassword)
{
    m_rplaybackrun.setUserVerifyInfo(sUsername,sPassword);
// 	m_sUserName = sUsername;
//  m_sUserPwd  = sPassword;
}

int   RPlaybackWnd::startSearchRecFile(int nChannel,int nTypes,const QString & startTime,const QString & endTime)
{
	if (nTypes < 0 || nTypes > 15){
		qDebug()<<__FUNCTION__<<__LINE__<<"input param nTypes error!";
		return 1;
	}
	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
	if (!start.isValid() || !end.isValid() || start >= end){
		qDebug()<<__FUNCTION__<<__LINE__<<"input param startTime or endTime are error!";
		return 1;
	}

	m_rplaybackrun.startSearchRecFile(nChannel,nTypes,start,end);
	return 0;
}


 QString RPlaybackWnd::GetNowPlayedTime()
 {
	 if (NULL == m_GroupPlayback||m_CurStatus==STATUS_STOP)
	 {
		 return "-1";
	 }
	 return m_rplaybackrun.GetNowPlayedTime();
 }

int   RPlaybackWnd::GroupPlay(int nTypes,const QString & startTime,const QString & endTime)
{
	if (startTime.isEmpty() || endTime.isEmpty() || nTypes > 15){
		return 1;
	}

	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
	if (!start.isValid() || !end.isValid() || start >= end){
		qDebug()<<__FUNCTION__<<__LINE__<<"input param startTime or endTime are error!";
		return 1;
	}
	m_CurStatus = STATUS_NORMAL_PLAY;

	return m_rplaybackrun.GroupPlay(nTypes, start, end);
}

int   RPlaybackWnd::GroupPause()
{
    int nRet = -1;
//     if (NULL != m_GroupPlayback)
//     {
//         nRet = m_GroupPlayback->GroupPause();
//     }
	nRet = m_rplaybackrun.GroupPause();
	m_lastStatus = (RemotePlayBackStatus)GetCurrentState();
	m_CurStatus=STATUS_PAUSE;

    return nRet;
}
int   RPlaybackWnd::GroupContinue()
{
    int nRet = -1;
//     if (NULL != m_GroupPlayback)
//     {
//         nRet = m_GroupPlayback->GroupContinue();
//     } 
	nRet = m_rplaybackrun.GroupContinue();
	m_CurStatus = m_lastStatus;
    return nRet;
}
int   RPlaybackWnd::GroupStop()
{
    int nRet = -1;
//     if (NULL != m_GroupPlayback)
//     {
//         nRet = m_GroupPlayback->GroupStop();
// 		_widList.clear();
//     } 
	nRet = m_rplaybackrun.GroupStop();
	m_PlaybackWnd[0].closeSuspensionWnd();
	m_CurStatus=STATUS_STOP;
    return nRet;
}
int  RPlaybackWnd::AudioEnabled(bool bEnable)
{
    int bRet = m_PlaybackWnd[0].AudioEnabled(bEnable);
	bIsOpenAudio=bEnable;
// 	if (NULL != m_GroupPlayback)
// 	{
// 		m_GroupPlayback->GroupSetVolume(0xAECBCA, &m_PlaybackWnd[m_nCurrentWnd]);
// 	}
	bRet = m_rplaybackrun.SetVolume(0xAECBCA, &m_PlaybackWnd[m_nCurrentWnd]);
    return bRet;
}
int   RPlaybackWnd::SetVolume(const unsigned int &uiPersent)
{
	int nRet = -1;
	m_uiPersent=uiPersent;
// 	if (NULL != m_GroupPlayback)
// 	{
// 		nRet = m_GroupPlayback->GroupSetVolume(uiPersent, &m_PlaybackWnd[m_nCurrentWnd]);
// 	}
	nRet = m_rplaybackrun.SetVolume(uiPersent, &m_PlaybackWnd[m_nCurrentWnd]);
	return nRet;
}
int   RPlaybackWnd::GroupSpeedFast() 
{
    int nRet = -1;
//     if (NULL != m_GroupPlayback)
//     {
//         nRet = m_GroupPlayback->GroupSpeedFast();
//     } 
	nRet = m_rplaybackrun.GroupSpeedFast();
	m_CurStatus=STATUS_FAST_PLAY;
    return nRet;
}
int   RPlaybackWnd::GroupSpeedSlow()
{
    int nRet = -1;
//     if (NULL != m_GroupPlayback)
//     {
//         nRet = m_GroupPlayback->GroupSpeedSlow();
//     } 
	nRet = m_rplaybackrun.GroupSpeedSlow();
	m_CurStatus=STATUS_SLOW_PLAY;
    return nRet;
}
int   RPlaybackWnd::GroupSpeedNormal()
{
    int nRet = -1;
//     if (NULL != m_GroupPlayback)
//     {
//         nRet = m_GroupPlayback->GroupSpeedNormal();
//     } 
	nRet = m_rplaybackrun.GroupSpeedNormal();
	m_CurStatus=STATUS_NORMAL_PLAY;
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
	 pRegist->registerEvent("foundFile",cbFoundFile,this);
	 pRegist->registerEvent("CurrentStatus",cbStateChange,this);
	 pRegist->registerEvent("recFileSearchFinished",cbRecFileSearchFinished,this);
// 	 pRegist->registerEvent("bufferStatus",cbCacheState,this);
     pRegist->registerEvent("recFileSearchFail",cbRecFileSearchFail,this);
	 pRegist->Release();
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
	QString key;
	key="index_";
	fileMap.insert(key.append(fileKey),fileinfo);
	int ifilekey;
	ifilekey=fileKey.toInt();
	fileKey=QString::number(ifilekey+1);
	if (fileTotal<100)
	{
		if (ifilekey==fileTotal-1)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<ifilekey<<fileTotal;
			emit FoundFileToUiS(fileMap);
		}
	}else{
		if (ifilekey%99==0&&ifilekey!=0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"ifilekey"<<ifilekey<<"fileTotal"<<fileTotal;
			fileTotal=fileTotal-100;
			fileKey="0";
			emit FoundFileToUiS(fileMap);
			fileMap.clear();
		}
		}
}

void RPlaybackWnd::RecFileSearchFinished( QVariantMap evMap )
{
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
	if (m_CurStatus<STATUS_PAUSE)
	{
		RSubView::showSusWnd(false);
		GroupPause();
		bIsHide=true;
	}else{
		//do nothing
	}
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
		if (bIsHide==true)
		{
			RSubView::showSusWnd(true);
			GroupContinue();
			bIsHide=false;
		}else{
			//do nothing
		}
		
	}else{
		GroupStop();
	}
	
	loadLauguage();
}



void RPlaybackWnd::screenShot( QString sUser,int nType )
{
	return m_rplaybackrun.screenShot(sUser,nType,m_nCurrentWnd);
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
	EventProcCall("recFileSearchFinished",evMap);
}

void RPlaybackWnd::SocketErrorToUislot( QVariantMap evMap )
{

}

void RPlaybackWnd::StateChangeToUislot( QVariantMap evMap )
{
	int status = evMap.value("CurrentStatus").toInt();
	for (int index = 0; index < ARRAY_SIZE(m_PlaybackWnd); ++index)
	{
		m_PlaybackWnd[index].SetCurConnectState((RSubView::__enConnectStatus)status);
	}
// 	_curConnectState=(__enConnectStatus)evMap.value("CurrentStatus").toInt();
// 	if (_curConnectType==TYPE_STREAM)
// 	{
// 		QList<int>::Iterator it;
// 		for(it=_widList.begin();it!=_widList.end();it++){
// 			m_PlaybackWnd[*it].SetCurConnectState((RSubView::__enConnectStatus)_curConnectState);
// 		}
// 	}
// 	if (_curConnectState==STATUS_DISCONNECTED)
// 	{
// 		_mutexWidList.lock();
// 		_widList.clear();
// 		_mutexWidList.unlock();
// 	}
}

void RPlaybackWnd::CacheStateToUislot( QVariantMap evMap )
{
	QList<int>::iterator it;
	for(it=_widList.begin();it!=_widList.end();it++){
		if (evMap.value("wind").toInt() == (int)&m_PlaybackWnd[*it])
		{
			m_PlaybackWnd[*it].CacheState(evMap);
		}
	}
}

int RPlaybackWnd::GetCurrentState()
{
	return m_CurStatus;
}
void RPlaybackWnd::FileSearchStartUislot( QVariantMap evMap)
{
	EventProcCall("recFileSearchStart",evMap);
}
void RPlaybackWnd::FileSearchFailUislot( QVariantMap evMap)
{
	EventProcCall("recFileSearchFail",evMap);
}

void RPlaybackWnd::RecFileSearchFail( QVariantMap evMap )
{
	emit FileSearchFailToUiS(evMap);
}

void RPlaybackWnd::loadLauguage()
{
	// Get language description from system
	QString sLang;
	ILocalSetting * pi = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void **)&pi);
	if (NULL != pi){
		sLang = pi->getLanguage();
		pi->Release();
	}else{
		sLang = QString("en_GB");
	}

	// Get language file pathname
	QString sLanguageConfigPath(QCoreApplication::applicationDirPath() + QString("/LocalSetting"));
	QString sLanguageConfigFile(sLanguageConfigPath + QString("/language.xml"));
	QDomDocument confFile;
	QFile *file = new QFile(sLanguageConfigFile);
	file->open(QIODevice::ReadOnly);
	confFile.setContent(file);
	QDomNode clsidNode = confFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	QString sFileName="en_GB";
	for (int i = 0; i < itemList.count(); i++){
		QDomNode item = itemList.at(i);
		QString slanguage = item.toElement().attribute("name");
		if(slanguage == sLang){
			sFileName =item.toElement().attribute("file");
			break;
		}
	}
	file->close();
	delete file;

	// load language file
	m_translator.load(sFileName,sLanguageConfigPath);
}

void RPlaybackWnd::slValidateFail( QVariantMap vmap )
{
	EventProcCall(QString("Validation"), vmap);
}

void RPlaybackWnd::slScreenShot( QVariantMap evMap)
{
	EventProcCall("screenShot",evMap);
}



/*
 int cbFoundFile(QString evName,QVariantMap evMap,void*pUser)
 {
     int nRet = 1;
     if (evName == "foundFile")
     {
		 ((RPlaybackWnd*)pUser)->FoundFile(evMap);
     }
     return nRet;
 }
 int cbRecFileSearchFail(QString evName,QVariantMap evMap,void*pUser)
 {
	 int nRet = 1;
	 if (evName == "recFileSearchFail")
	 {
		 ((RPlaybackWnd*)pUser)->RecFileSearchFail(evMap);
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
 */

void cbDigitalZoom( QString evName, QVariantMap item, void* pUser )
{
	((PlayBackThread*)pUser)->setInfromation(evName, item);
}
