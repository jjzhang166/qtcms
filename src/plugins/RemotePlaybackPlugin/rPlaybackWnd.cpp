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
bIsCaseInitFlags(false)
// m_DeviceClient(NULL)
{
	for (int i = 0; i < ARRAY_SIZE(m_PlaybackWnd); ++i)
	{
		m_PlaybackWnd[i].setParent(this);
		connect(&m_PlaybackWnd[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_PlaybackWnd[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));
		connect(&m_PlaybackWnd[i], SIGNAL(ChangeAudioHint(QString, RSubView*)), this, SLOT(ChangeAudioHint(QString, RSubView*)));

		m_PlaybackWndList.insert(m_PlaybackWndList.size(),&m_PlaybackWnd[i]);
	}

	pcomCreateInstance(CLSID_DivMode2_2,NULL,IID_IWindowDivMode,(void **)&m_DivMode);
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->setParentWindow(this);
  		m_DivMode->setSubWindows(m_PlaybackWndList,ARRAY_SIZE(m_PlaybackWnd));
  		m_DivMode->flush();
 	}
	m_RemotePlaybackObject.SetrPlaybackWnd(this);
}

RPlaybackWnd::~RPlaybackWnd()
{
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->Release();
 		m_DivMode = NULL;
 	}
//     if (m_DeviceClient != NULL)
//     {
//         m_DeviceClient->Release();
//         m_DeviceClient = NULL;
//     }
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
    if (vendor.isEmpty())
    {
        return 1;
    }
    m_sVendor = vendor;
    return 0;
}

int   RPlaybackWnd::AddChannelIntoPlayGroup(uint uiWndId,unsigned int uiChannel)
{
	qDebug()<<"RPlaybackWnd AddChannelIntoPlayGroup"<<uiWndId<<uiChannel;
    if (uiWndId >= ARRAY_SIZE(m_PlaybackWnd) || uiChannel > 32)
    {
        return -1;
    }
    int nRet = 2;

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
        nRet = m_GroupPlayback->AddChannelIntoPlayGroup(uiChannel, &m_PlaybackWnd[uiWndId]);
		_widList<<uiWndId;
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
    m_sUserName = sUsername;
    m_sUserPwd  = sPassword;
}

int   RPlaybackWnd::startSearchRecFile(int nChannel,int nTypes,const QString & startTime,const QString & endTime)
{
	qDebug()<<"RPlaybackWnd:"<<"nChannel"<<nChannel<<"nTypes"<<nTypes<<startTime<<endTime;
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
	nRet=m_RemotePlaybackObject.SetParm(m_sUserName,m_sUserPwd,m_uiPort,m_sHostAddress,m_sEseeId);
	if (1==nRet)
	{
		goto finishSearch;
	}
	_curConnectType=TYPE_SEARCH;
	nRet=m_RemotePlaybackObject.startSearchRecFile(nChannel,nTypes,startTime,endTime);
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
     if (NULL != m_GroupPlayback )
     {
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
     else
         return NULL;
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
	nRet=m_RemotePlaybackObject.SetParm(m_sUserName,m_sUserPwd,m_uiPort,m_sHostAddress,m_sEseeId);
	if (1==nRet)
	{
		return nRet;
	}
	_curConnectType=TYPE_STREAM;
	nRet=m_RemotePlaybackObject.GroupPlay(nTypes,startTime,endTime);
	QList<int>::Iterator it;
	for(it=_widList.begin();it!=_widList.end();it++){
		m_PlaybackWnd[*it].saveCacheImage();
	}
	return nRet;
}

int   RPlaybackWnd::GroupPause()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupPause();
    } 
    return nRet;
}
int   RPlaybackWnd::GroupContinue()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupContinue();
    } 
    return nRet;
}
int   RPlaybackWnd::GroupStop()
{
	qDebug()<<"RPlaybackWnd:GroupStop";
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupStop();
    } 
    return nRet;
}
// bool  RPlaybackWnd::GroupEnableAudio(bool bEnable)
// {
//     bool bRet = false;
//     if (NULL != m_GroupPlayback)
//     {
//         bRet = m_GroupPlayback->GroupEnableAudio(bEnable);
//     } 
//     return bRet;
// }
int   RPlaybackWnd::GroupSetVolume(const unsigned int &uiPersent)
{
	int nRet = -1;
	if (NULL != m_GroupPlayback)
	{
		nRet = m_GroupPlayback->GroupSetVolume(uiPersent, NULL);
	}
	return 0;
}
int   RPlaybackWnd::SetVolume(const unsigned int &uiPersent)
{
    bool bRet = m_PlaybackWnd[0].AudioEnabled(bEnable);
    return bRet;
}
int   RPlaybackWnd::GroupSetVolume(const unsigned int &uiPersent)
{
	int nRet = -1;
	if (NULL != m_GroupPlayback)
	{
		nRet = m_GroupPlayback->GroupSetVolume(uiPersent, NULL);
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
    return nRet;
}
int   RPlaybackWnd::GroupSpeedSlow()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupSpeedSlow();
    } 
    return nRet;
}
int   RPlaybackWnd::GroupSpeedNormal()
{
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupSpeedNormal();
    } 
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
            break;
        }
    }
    m_nCurrentWnd=j;
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
	qDebug()<<evMap;
	EventProcCall("RecFileInfo",evMap);
}

void RPlaybackWnd::RecFileSearchFinished( QVariantMap evMap )
{
	qDebug()<<evMap;
	EventProcCall("recFileSearchFinished",evMap);
}

void RPlaybackWnd::SocketError( QVariantMap evMap )
{

}

void RPlaybackWnd::StateChange( QVariantMap evMap )
{
	qDebug()<<evMap;
	_curConnectState=(__enConnectStatus)evMap.value("CurrentStatus").toInt();
	if (_curConnectState==STATUS_DISCONNECTED)
	{
		_widList.clear();
	}
	if (_curConnectType==TYPE_STREAM)
	{
		QList<int>::Iterator it;
		for(it=_widList.begin();it!=_widList.end();it++){
			m_PlaybackWnd[*it].SetCurConnectState((RSubView::__enConnectStatus)_curConnectState);
		}
	}
}
void RPlaybackWnd::CacheState( QVariantMap evMap )
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
     qDebug()<<"cbRecFileSearchFinished";
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
 i++;
 }
 return 0;
 }*/
// void analyze(QList<RecordInfo>& lstInfo, QList<TimeInfo>&lstTimeInfo)
// {
//     QDateTime start, end;
//	 for (int i = 0; i < lstInfo.size(); ++i)
//	 {
//		 RecordInfo record = lstInfo.at(i);
//		 int index = checkChannel(lstTimeInfo, record.uiChannel);
//		 if (0 == index)
//		 {
//			 TimeInfo ti;
//			 Session se;
//			 ti.uiChannel = record.uiChannel;
//			 se.uiType = record.uiTypes;
//			 se.timeSession<<record.sStartTime<<record.sEndTime;
//			 ti.lstTypeTime.append(se);
//			 lstTimeInfo.append(ti);
//			 continue;
//		 }
//		 else
//		 {
//			 int in = checkType(lstTimeInfo[index].lstTypeTime, record.uiTypes);
//			 if (0 == in)
//			 {
//				 Session se;
//				 se.uiType = record.uiTypes;
//				 se.timeSession<<record.sStartTime<<record.sEndTime;
//				 lstTimeInfo[index].lstTypeTime.append(se);
//				 continue;
//			 }
//			 else
//			 {
//                 bool bExistFlag = false;
//                 for (int j = 0; j < lstTimeInfo[index].lstTypeTime[in].timeSession.size(); j+=2)
//                 {
//                     start = lstTimeInfo[index].lstTypeTime[in].timeSession.at(j);
//                     end   = lstTimeInfo[index].lstTypeTime[in].timeSession.at(j + 1);
//                     if (end.addSecs(1) == record.sStartTime)
//                     {
//                         bExistFlag = false;
//                         lstTimeInfo[index].lstTypeTime[in].timeSession.replace(j + 1, record.sEndTime);
//                         break;
//                     }
//                     else if (record.sEndTime.addSecs(1) == start)
//                     {
//                         bExistFlag = false;
//                         lstTimeInfo[index].lstTypeTime[in].timeSession.replace(j, record.sStartTime);
//                         break;
//                     }
//                     else 
//                     {
//                         bExistFlag = true;
//                     }
//                 }
//                 if (bExistFlag)
//                 {
//                     lstTimeInfo[index].lstTypeTime[in].timeSession.append(record.sStartTime);
//                     lstTimeInfo[index].lstTypeTime[in].timeSession.append(record.sEndTime);
//                 }
//			 }
//		 }
//	 }
//}


void RPlaybackWnd::ChangeAudioHint(QString statement, RSubView* pWind)
{
	int index = pWind - m_PlaybackWnd;
	m_PlaybackWnd[index].setAudioHint(statement);
}
