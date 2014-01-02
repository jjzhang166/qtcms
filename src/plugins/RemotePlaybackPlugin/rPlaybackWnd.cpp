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
m_DeviceClient(NULL)
{
	for (int i = 0; i < ARRAY_SIZE(m_PlaybackWnd); ++i)
	{
		m_PlaybackWnd[i].setParent(this);
		connect(&m_PlaybackWnd[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_PlaybackWnd[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));
		m_PlaybackWndList.insert(m_PlaybackWndList.size(),&m_PlaybackWnd[i]);
	}

	pcomCreateInstance(CLSID_DivMode2_2,NULL,IID_IWindowDivMode,(void **)&m_DivMode);
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->setParentWindow(this);
  		m_DivMode->setSubWindows(m_PlaybackWndList,ARRAY_SIZE(m_PlaybackWnd));
  		m_DivMode->flush();
 	}
}

RPlaybackWnd::~RPlaybackWnd()
{
 	if (m_DivMode != NULL)
 	{
 		m_DivMode->Release();
 		m_DivMode = NULL;
 	}
    if (m_DeviceClient != NULL)
    {
        m_DeviceClient->Release();
        m_DeviceClient = NULL;
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
    if (!m_HostAddress.setAddress(sAddress) || uiPort > 65535)
    {
        return 1;
    }

    m_uiPort   = uiPort;
    m_sEseeId  = eseeID;
    return 0;
}

int   RPlaybackWnd::setDeviceVendor(const QString & vendor)
{
    if (vendor.isEmpty())
    {
        return 1;
    }
    m_sVendor = vendor;
    return 0;
}

int   RPlaybackWnd::AddChannelIntoPlayGroup(uint uiWndId,unsigned int uiChannel)
{
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
                bIsCaseInitFlags = true;
                break;
            }
        }
        file->close();
        delete file;
    }
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->AddChannelIntoPlayGroup(uiChannel, m_PlaybackWnd[uiWndId].ui->widget_display);
    }
    else
    {
        bIsCaseInitFlags = false;
    }
    return nRet;
}

void   RPlaybackWnd::setUserVerifyInfo(const QString & sUsername,const QString & sPassword)
{
    m_sUserName = sUsername;
    m_sUserPwd  = sPassword;
}

int   RPlaybackWnd::startSearchRecFile(int nChannel,int nTypes,const QString & startTime,const QString & endTime)
{
	int nRet = -1;
    g_RecList.clear();
    m_SelectedRecList.clear();
    bIsCaseInitFlags = false;
	if (NULL != m_GroupPlayback && !startTime.isEmpty() && !endTime.isEmpty())
	{
        if (m_DeviceClient != NULL)
        {
            m_DeviceClient->Release();
            m_DeviceClient = NULL;
        }
        m_GroupPlayback->QueryInterface(IID_IDeviceClient , (void **)&m_DeviceClient);
        if (NULL == m_DeviceClient)
        {
            return nRet = 1;
        }
        if (!bIsInitFlags)
        {
            if (1 == cbInit())
            {
                return nRet = 2;
            }
        }
        m_DeviceClient->checkUser(m_sUserName,m_sUserPwd);
        m_DeviceClient->connectToDevice(m_HostAddress.toString(),m_uiPort,m_sEseeId);

        IDeviceSearchRecord *pDeviceSearchRecord = NULL;
        m_GroupPlayback->QueryInterface(IID_IDeviceSearchRecord, (void**)&pDeviceSearchRecord);
        if (NULL == pDeviceSearchRecord)
        {
            return nRet;
        }
        QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
        QDateTime end   = QDateTime::fromString(endTime,   "yyyy-MM-dd hh:mm:ss");
        nRet = pDeviceSearchRecord->startSearchRecFile(nChannel, nTypes, start, end);
        
        QFuture<int> future = QtConcurrent::run(childThreadSearch,m_uiRecFileSearched, startTime, endTime, m_SelectedRecList);
        nRet = future.result();
        future.waitForFinished();
        for (int i = 0; i < m_SelectedRecList.size(); ++i)
        {
            EventProcCall("RecFileInfo",m_SelectedRecList[i]);
        }
        m_SelectedRecList.clear();
        pDeviceSearchRecord->Release();
        pDeviceSearchRecord = NULL;
        m_DeviceClient->Release();
        m_DeviceClient = NULL;
	}
	return nRet;
}
int  childThreadSearch(uint nNum, QString& start,QString& end, QList<QVariantMap> &selectedList)
{
    int nRet = -1;
    if (nNum != g_RecList.size())
    {
        QElapsedTimer t;
        t.start();
        while(t.elapsed()<2000)
            QCoreApplication::processEvents();
    }
    QVariantMap vMap;
    uint uiChannel = 0;
    uint uiType    = 0;
  	QList<TimeInfo> lstTimeInfo;
    analyze(g_RecList, lstTimeInfo);
    for (int i = 0; i < lstTimeInfo.size(); ++i)
    {
        uiChannel = lstTimeInfo.at(i).uiChannel;
        for (int j = 0; j < lstTimeInfo.at(i).lstTypeTime.size(); ++j)
        {
            uiType = lstTimeInfo.at(i).lstTypeTime.at(j).uiType;
            for (int k = 0; k < lstTimeInfo.at(i).lstTypeTime.at(j).timeSession.size(); k+=2)
            {
                vMap.clear();
                vMap.insert("channel", uiChannel);
                vMap.insert("types", uiType);
                vMap.insert("start", lstTimeInfo.at(i).lstTypeTime.at(j).timeSession.at(k));
                vMap.insert("end",lstTimeInfo.at(i).lstTypeTime.at(j).timeSession.at(k+1));
                selectedList.append(vMap);
            }
        }
    }
	return nRet = 0;
}

 QString RPlaybackWnd::getGroupPlayedTime()
 {
     if (NULL != m_GroupPlayback )
     {
         QDateTime playedTime = m_GroupPlayback->GroupGetPlayedTime();
         return playedTime.toString("yyyy-MM-dd hh:mm:ss");
     }
     else
         return NULL;
 }

int   RPlaybackWnd::GroupPlay(int nTypes,const QString & startTime,const QString & endTime)
{
    int nRet = -1;
    if (NULL != m_GroupPlayback && !startTime.isEmpty() && !endTime.isEmpty() )
    {
        if (m_DeviceClient != NULL)
        {
            m_DeviceClient->Release();
            m_DeviceClient = NULL;
        }
        m_GroupPlayback->QueryInterface(IID_IDeviceClient , (void **)&m_DeviceClient);
        if (NULL == m_DeviceClient)
        {
            return nRet = -2;
        }
        if (!bIsInitFlags)
        {
            if (1 == cbInit())
            {
                nRet = 3;
            }
        }
        m_DeviceClient->connectToDevice(m_HostAddress.toString(),m_uiPort,m_sEseeId);

		QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
		QDateTime end   = QDateTime::fromString(endTime,   "yyyy-MM-dd hh:mm:ss");
        nRet = m_GroupPlayback->GroupPlay(nTypes, start, end);

        if (m_DeviceClient != NULL)
        {
            m_DeviceClient->Release();
            m_DeviceClient = NULL;
        }
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
    int nRet = -1;
    if (NULL != m_GroupPlayback)
    {
        nRet = m_GroupPlayback->GroupStop();
    } 
    return nRet;
}
bool  RPlaybackWnd::GroupEnableAudio(bool bEnable)
{
    bool bRet = false;
    if (NULL != m_GroupPlayback)
    {
        bRet = m_GroupPlayback->GroupEnableAudio(bEnable);
    } 
    return bRet;
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
     evName.append("StateChangeed");
     pRegist->registerEvent(evName,cbStateChange,this);
     evName.clear();
     evName.append("recFileSearchFinished");
     pRegist->registerEvent(evName,cbRecFileSearchFinished,this);
     pRegist->Release();
     pRegist=NULL;

     bIsInitFlags=true;
     return 0;
 }

 int cbFoundFile(QString evName,QVariantMap evMap,void*pUser)
 {
     int nRet = 1;
     if (evName == "foundFile")
     {
         RecordInfo recordInfo;
         recordInfo.uiChannel = evMap["channel"].toUInt();
         recordInfo.uiTypes = evMap["types"].toUInt();
         recordInfo.sStartTime = QDateTime::fromString(evMap["start"].toString(), "yyyy-MM-dd hh:mm:ss");
         recordInfo.sEndTime = QDateTime::fromString(evMap["end"].toString(), "yyyy-MM-dd hh:mm:ss");
         recordInfo.sFileName = evMap["filename"].toString();

		 qDebug()<<recordInfo.uiChannel<<","<<recordInfo.uiTypes<<","<<recordInfo.sStartTime<<","<<recordInfo.sEndTime<<","<<recordInfo.sFileName;
         g_RecList.append(recordInfo);
         nRet = 0;
     }
     return nRet;

 }

 int cbRecFileSearchFinished(QString evName,QVariantMap evMap,void*pUser)
 {
     int nRet = 1;
     qDebug()<<"cbRecFileSearchFinished";
     if (evName == "recFileSearchFinished")
     {
         QVariantMap::const_iterator it;
         for (it=evMap.begin();it!=evMap.end();++it)
         {
             ((RPlaybackWnd*)pUser)->GetRecFileNum(it.value().toUInt());
             qDebug()<<"total"<<it.value().toUInt();
             nRet = 0;
         }
     }
     return nRet;
 }

 int cbStateChange(QString evName,QVariantMap evMap,void*pUser)
 {
     qDebug()<<"cbStateChange";
     if (evName=="StateChangeed")
     {
         ((RPlaybackWnd*)pUser)->CurrentStateChangePlugin(evMap.value("status").toInt());
         return 0;
     }
     return 1;
 }
 int checkChannel(QList<TimeInfo>& lstTimeInfo, uint& uiChannel)
 {
	 int i = 0;
     int nTmp = lstTimeInfo.size();
	 while(i < nTmp)
	 {
		 if (lstTimeInfo[i].uiChannel == uiChannel)
		 {
			 return i;
		 }
		 i++;
	 }
	 return 0;
 }
 int checkType(QList<Session>& lstTypeTime, uint& uiType)
 {
	 int i = 0;
     int nTmp = lstTypeTime.size();
	 while(i < nTmp)
	 {
		 if (lstTypeTime[i].uiType == uiType)
		 {
			 return i;
		 }
		 i++;
	 }
	 return 0;
 }
 void analyze(QList<RecordInfo>& lstInfo, QList<TimeInfo>&lstTimeInfo)
 {
     QDateTime start, end;
	 for (int i = 0; i < lstInfo.size(); ++i)
	 {
		 RecordInfo record = lstInfo.at(i);
		 int index = checkChannel(lstTimeInfo, record.uiChannel);
		 if (0 == index)
		 {
			 TimeInfo ti;
			 Session se;
			 ti.uiChannel = record.uiChannel;
			 se.uiType = record.uiTypes;
			 se.timeSession<<record.sStartTime<<record.sEndTime;
			 ti.lstTypeTime.append(se);
			 lstTimeInfo.append(ti);
			 continue;
		 }
		 else
		 {
			 int in = checkType(lstTimeInfo[index].lstTypeTime, record.uiTypes);
			 if (0 == in)
			 {
				 Session se;
				 se.uiType = record.uiTypes;
				 se.timeSession<<record.sStartTime<<record.sEndTime;
				 lstTimeInfo[index].lstTypeTime.append(se);
				 continue;
			 }
			 else
			 {
                 bool bExistFlag = false;
                 for (int j = 0; j < lstTimeInfo[index].lstTypeTime[in].timeSession.size(); j+=2)
                 {
                     start = lstTimeInfo[index].lstTypeTime[in].timeSession.at(j);
                     end   = lstTimeInfo[index].lstTypeTime[in].timeSession.at(j + 1);
                     if (end.addSecs(1) == record.sStartTime)
                     {
                         bExistFlag = false;
                         lstTimeInfo[index].lstTypeTime[in].timeSession.replace(j + 1, record.sEndTime);
                         break;
                     }
                     else if (record.sEndTime.addSecs(1) == start)
                     {
                         bExistFlag = false;
                         lstTimeInfo[index].lstTypeTime[in].timeSession.replace(j, record.sStartTime);
                         break;
                     }
                     else 
                     {
                         bExistFlag = true;
                     }
                 }
                 if (bExistFlag)
                 {
                     lstTimeInfo[index].lstTypeTime[in].timeSession.append(record.sStartTime);
                     lstTimeInfo[index].lstTypeTime[in].timeSession.append(record.sEndTime);
                 }
			 }
		 }
	 }
}
