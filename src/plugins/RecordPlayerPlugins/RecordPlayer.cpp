#include "RecordPlayer.h"
#include "libpcom.h"
#include "qwfw.h"
#include <QtGui/QResizeEvent>
#include <qwfw_tools.h>
#include <guid.h>
#include "IEventRegister.h"
#include "ILocalRecordSearchEx.h"
#include "ILocalPlayerEx.h"
#include <QApplication>
#include "ILocalSetting.h"
#include <QDomDocument>
#include <QDebug>

RecordPlayer::RecordPlayer():
QWebPluginFWBase(this),
m_pLocalRecordSearch(NULL),
m_pLocalPlayer(NULL),
m_pWindowDivMode(NULL),
m_currentWindID(0),
m_bIsOpenAudio(false),
m_bIsHide(false),
m_uiPersent(50),
m_wndNum(0),
m_wndCount(0),
m_lastStatus(STATUS_STOP),
m_CurStatus(STATUS_STOP)
{
	//申请ILocalRecordSearch接口
// 	pcomCreateInstance(CLSID_LocalPlayer,NULL,IID_ILocalRecordSearch,(void **)&m_pLocalRecordSearch);
	pcomCreateInstance(CLSID_LocalPlayerEx,NULL,IID_ILocalRecordSearchEx,(void **)&m_pLocalRecordSearch);

	//申请ILocalPlayer接口
// 	pcomCreateInstance(CLSID_LocalPlayer,NULL,IID_ILocalPlayer,(void **)&m_pLocalPlayer);
	//申请IWindowDivMode接口
	pcomCreateInstance(CLSID_DivMode2_2,NULL,IID_IWindowDivMode,(void **)&m_pWindowDivMode);

	//申请ILocalPlayer接口
	if (NULL != m_pLocalRecordSearch)
	{
		m_pLocalRecordSearch->QueryInterface(IID_ILocalPlayerEx, (void**)&m_pLocalPlayer);
		m_pLocalRecordSearch->QueryInterface(IID_IScreenShotDevice,(void**)&m_pScreenShotDevice);
	}


	for (int i = 0; i < ARRAY_SIZE(m_subRecPlayerView); i++)
	{
		m_subRecPlayerView[i].setParent(this);
		connect(&m_subRecPlayerView[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_subRecPlayerView[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));
		connect(&m_subRecPlayerView[i],SIGNAL(sigValidateFail(QVariantMap)), this, SLOT(slValidateFail(QVariantMap)));
		m_lstRecordPlayerWndList.insert(i,&m_subRecPlayerView[i]);

		m_subRecPlayerView[i].setLocalPlayer(m_pLocalPlayer);
	}
	m_subRecPlayerView[0].SetFocus(true);
	if (m_pWindowDivMode != NULL)
	{
		m_pWindowDivMode->setParentWindow(this);
		m_pWindowDivMode->setSubWindows(m_lstRecordPlayerWndList,ARRAY_SIZE(m_subRecPlayerView));
		m_pWindowDivMode->flush();
	}

	//注册事件
	cbInit();

	// Language
	QApplication::installTranslator(&m_translator);
}

RecordPlayer::~RecordPlayer()
{
	m_subRecPlayerView[0].destroySusWnd();
	if (NULL != m_pLocalRecordSearch)
	{
		m_pLocalRecordSearch->Release();
		m_pLocalRecordSearch = NULL;
	}
	if (NULL != m_pLocalPlayer)
	{
		m_pLocalPlayer->Release();
		m_pLocalPlayer = NULL;
	}
	if (NULL != m_pWindowDivMode)
	{
		m_pWindowDivMode->Release();
		m_pWindowDivMode = NULL;
	}
	if (NULL!=m_pScreenShotDevice)
	{
		m_pScreenShotDevice->Release();
		m_pScreenShotDevice=NULL;
	}
}

int RecordPlayer::cbInit()
{
	IEventRegister *pEvRegister = NULL;
	if (NULL == m_pLocalRecordSearch)
	{
		return 1;
	}

	m_pLocalRecordSearch->QueryInterface(IID_IEventRegister, (void**)&pEvRegister);
	if (NULL == pEvRegister)
	{
		return 1;
	}

	pEvRegister->registerEvent(QString("GetRecordDate"), cbGetRecordDate, this);
	pEvRegister->registerEvent(QString("GetRecordFile"), cbGetRecordFile, this);
	pEvRegister->registerEvent(QString("SearchStop"), cbSearchStop, this);
	pEvRegister->registerEvent(QString("GetRecordFileEx"), cbGetRecordFile, this);
	pEvRegister->registerEvent(QString("ThrowException"), cbThrowException, this);
	pEvRegister->registerEvent(QString("screenShot"),cbScreenShot,this);
	pEvRegister->Release();
	return 0;
}

void  RecordPlayer::resizeEvent(QResizeEvent *ev)
{
	if (NULL != m_pWindowDivMode)
	{
		m_pWindowDivMode->parentWindowResize(ev);
	}
}

void  RecordPlayer::OnSubWindowDblClick(QWidget *wind,QMouseEvent *ev)
{
	if (NULL==m_pWindowDivMode)
	{
		return ;
	}
	m_pWindowDivMode->subWindowDblClick(wind,ev);
}

void  RecordPlayer::SetCurrentWind(QWidget *wind)
{
	int j;
	for (j = 0; j < ARRAY_SIZE(m_subRecPlayerView); ++j)
	{
		if (&m_subRecPlayerView[j] == wind)
		{
			m_subRecPlayerView[j].SetFocus(true);
			m_currentWindID = j;
		}else{
			m_subRecPlayerView[j].SetFocus(false);
		}
	}

}

int RecordPlayer::GetCurrentWnd()
{
	return m_currentWindID;
}

// int RecordPlayer::searchDateByDeviceName(const QString& sdevname)
// {
// 	qDebug()<<"RecordPlayer :searchDateByDeviceName:"<<sdevname;
// 	m_devicename=sdevname;
// 	if (sdevname.isEmpty())
// 	{
// 		return 1;
// 	}
// 
// 	if (NULL == m_pLocalRecordSearch)
// 	{
// 		return 1;
// 	}
// 
// 	int nRet = m_pLocalRecordSearch->searchDateByDeviceName(sdevname);
// 	if (ILocalRecordSearch::OK != nRet)
// 	{
// 		return 1;
// 	}
// 
// 	return 0;
// }
// int RecordPlayer::searchVideoFile(const QString& sdevname,
// 	const QString& sdate,
// 	const QString& sbegintime,
// 	const QString& sendtime,
// 	const QString& schannellist)
// {
// 	qDebug()<<"RecordPlayer :searchVideoFile:"<<sdevname<<sbegintime<<sendtime<<schannellist;
// 	fileMap.clear();
// 	fileKey="0";
// 	if (NULL == m_pLocalRecordSearch)
// 	{
// 		return 1;
// 	}
// 
// 	int nRet = m_pLocalRecordSearch->searchVideoFile(sdevname, sdate, sbegintime, sendtime, schannellist);
// 	if (ILocalRecordSearch::OK != nRet)
// 	{
// 		return 1;
// 	}
// 	EventProcCall("GetRecordFile",fileMap);
// 	return 0;
// }

QDateTime RecordPlayer::getDateFromPath(QString &filePath)
{
	QDateTime dateTime;;
	QDate date;
	QTime time;
	QString dateStr;
	QString timeStr;
	QRegExp rx("([0-9]{4}-[0-9]{2}-[0-9]{2})");

	if (-1 != rx.indexIn(filePath,0))
	{
		dateStr = rx.cap(1);
	}

	rx = QRegExp("([0-9]{6}).avi");
	if (-1 != rx.indexIn(filePath, 0))
	{
		timeStr = rx.cap(1);
	}
	date = QDate::fromString(dateStr, "yyyy-MM-dd");
	time = QTime::fromString(timeStr, "hhmmss");
	dateTime.setDate(date);
	dateTime.setTime(time);

	return dateTime;
}

int RecordPlayer::sortFileList(QStringList &fileList)
{
	QDateTime minDate;
	QDateTime tempDate;
	int keyPos = 0;
	for (int i = 0; i < fileList.size() - 1; i++)
	{
		if (!QFile::exists(fileList[i]))
		{
			return -1;
		}
		keyPos = i;
		minDate = getDateFromPath(fileList[i]);
		for (int j = i + 1; j < fileList.size(); j++)
		{
			tempDate = getDateFromPath(fileList[j]);
			if (minDate > tempDate)
			{
				minDate = tempDate;
				keyPos = j;
			}
		}
		if (i != keyPos)
		{
			fileList.swap(i, keyPos);
		}
	}
	return 0;
}

// int RecordPlayer::AddFileIntoPlayGroup(const QString &filelist,const int &nWndID,const QString &startTime,const QString &endTime)
// {
// 	if (NULL == m_pLocalPlayer || filelist.isEmpty() || nWndID <0 || nWndID >= ARRAY_SIZE(m_subRecPlayerView))
// 	{
// 		return 1;
// 	}
// 
// 	QStringList lstFileList = filelist.split(",", QString::SkipEmptyParts);
// 	if (-1 == sortFileList(lstFileList))
// 	{
// 		return 1;
// 	}
// 
// 	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
// 	QDateTime end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
// 
// 	int nRet = m_pLocalPlayer->AddFileIntoPlayGroup(lstFileList, &m_subRecPlayerView[nWndID], start, end);
// 	if (1 == nRet)
// 	{
// 		return 1;
// 	}
// 
// 	return 0;
// }

int RecordPlayer::SetSynGroupNum(int num)
{
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->SetSynGroupNum(num);
	if (1 == nRet)
	{
		return 1;
	}
	
	return 0;
}
int RecordPlayer::GroupPlay()
{
	qDebug()<<"RecordPlayer :GroupPlay:";
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupPlay();
	if (1 == nRet)
	{
		return 1;
	}
	/*SetVolume(0xAECBCA);*/
// 	AudioEnabled(m_bIsOpenAudio);
// 	SetVolume(m_uiPersent);
	m_CurStatus=STATUS_NORMAL_PLAY;
	RecordPlayerView::setPlayStatus(m_CurStatus);
	return 0;
}
int RecordPlayer::GroupPause()
{
	qDebug()<<"RecordPlayer :GroupPause:";
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupPause();
	if (1 == nRet)
	{
		return 1;
	}
	m_lastStatus = (RecordPlayStatus)GetCurrentState();
	m_CurStatus=STATUS_PAUSE;
	return 0;
}
int RecordPlayer::GroupContinue()
{
	qDebug()<<"RecordPlayer :GroupContinue:";
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupContinue();
	if (1 == nRet)
	{
		return 1;
	}
	m_CurStatus = m_lastStatus;
	return 0;
}
int RecordPlayer::GroupStop()
{
	qDebug()<<"RecordPlayer :GroupStop:";
	if (NULL == m_pLocalPlayer /*|| STATUS_STOP == m_CurStatus*/)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupStop();
	if (1 == nRet)
	{
		return 1;
	}
	m_CurStatus=STATUS_STOP;
	RecordPlayerView::setPlayStatus(m_CurStatus);
	m_subRecPlayerView[0].closeSuspensionWnd();

	//clear up residual picture when stop
	for (int i = 0; i < ARRAY_SIZE(m_subRecPlayerView); i++)
	{
		m_subRecPlayerView[i].update();
		m_subRecPlayerView[i].setPlayingFlag(false);
	}
	m_wndNum = 0;

	return 0;
}
int RecordPlayer::GroupSpeedFast(int speed)
{
	qDebug()<<"RecordPlayer :GroupSpeedFast:"<<speed;
	if (NULL == m_pLocalPlayer || speed < 0)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupSpeedFast(speed);
	if (1 == nRet)
	{
		return 1;
	}
	m_CurStatus=STATUS_FAST_PLAY;
	return 0;
}
int RecordPlayer::GroupSpeedSlow(int speed)
{
	qDebug()<<"RecordPlayer :GroupSpeedSlow:"<<speed;
	if (NULL == m_pLocalPlayer || speed < 0)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupSpeedSlow(speed);
	if (1 == nRet)
	{
		return 1;
	}
	m_CurStatus=STATUS_SLOW_PLAY;
	return 0;
}
int RecordPlayer::GroupSpeedNormal()
{
	qDebug()<<"RecordPlayer :GroupSpeedNormal:";
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupSpeedNormal();
	if (1 == nRet)
	{
		return 1;
	}
	m_CurStatus=STATUS_NORMAL_PLAY;
	return 0;
}

QString RecordPlayer::GetNowPlayedTime()
{
	QString playedTime = "-1";
	if (NULL == m_pLocalPlayer||m_CurStatus==STATUS_STOP)
	{
		return playedTime;
	}

	QDateTime curDateTime = m_pLocalPlayer->GetNowPlayedTime();
	QTime curTime = curDateTime.time();
	QTime baseTime(0, 0, 0);
	playedTime = QString::number(baseTime.secsTo(curTime));

	return playedTime;
}
int RecordPlayer::AudioEnabled(bool bEnabled)
{
	m_bIsOpenAudio=bEnabled;
	return m_subRecPlayerView[m_currentWindID].AudioEnabled(bEnabled);
}
int RecordPlayer::SetVolume(const unsigned int &uiPersent)
{
	int nRet = -1;
	m_uiPersent=uiPersent;
	if (NULL != m_pLocalPlayer)
	{
		nRet = m_pLocalPlayer->GroupSetVolume(uiPersent, &m_subRecPlayerView[m_currentWindID]);
	}
	return nRet;
}
int cbGetRecordDate(QString evName,QVariantMap evMap,void*pUser)
{
	RecordPlayer *pRecordPlayer = (RecordPlayer*)pUser;
	if ("GetRecordDate" == evName)
	{
		pRecordPlayer->transRecordDate(evMap);
	}
	return 0;
}
int cbGetRecordFile(QString evName,QVariantMap evMap,void*pUser)
{
	RecordPlayer *pRecordPlayer = (RecordPlayer*)pUser;
	if ("GetRecordFile" == evName)
	{
		pRecordPlayer->transRecordFiles(evMap);

		qDebug()<<evMap["channelnum"].toInt()<<evMap["startTime"].toDateTime().toString("hh:mm:ss")<<evMap["stopTime"].toDateTime().toString("hh:mm:ss")<<evMap["filepath"].toString();
	}
	if ("GetRecordFileEx" == evName)
	{
		SearchProcess *pSchProc = pRecordPlayer->getCurProc(evMap.value("wndId").toInt());
		pSchProc->transRecordFilesEx(evMap);
	}
	return 0;
}
int cbSearchStop(QString evName,QVariantMap evMap,void*pUser)
{
	RecordPlayer *pRecordPlayer = (RecordPlayer*)pUser;
	if ("SearchStop" == evName)
	{
		pRecordPlayer->transSearchStop(evMap);
	}
	return 0;
}
int cbThrowException(QString evName, QVariantMap evMap, void* pUser)
{
	RecordPlayer *pRecordPlayer = (RecordPlayer*)pUser;
	if ("ThrowException" == evName)
	{
		pRecordPlayer->throwException(evMap);
	}
	return 0;
}

int cbScreenShot( QString evName, QVariantMap evMap, void* pUser )
{
	RecordPlayer *pRecordPlayer = (RecordPlayer*)pUser;
	if ("screenShot" == evName)
	{
		pRecordPlayer->transScreenShot(evMap);
	}
	return 0;
}

void RecordPlayer::transRecordDate(QVariantMap &evMap)
{
	QDateTime date = evMap["date"].toDateTime();

	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"devname",evMap["devname"].toString());
	EP_ADD_PARAM(arg,"date",date.toString("yyyy-MM-dd"));
	EventProcCall("GetRecordDate",arg);
}
void RecordPlayer::transRecordFiles(QVariantMap &evMap)
{
	QDateTime startTime = evMap["startTime"].toDateTime();
	QDateTime stopTime = evMap["stopTime"].toDateTime();

	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"filename",evMap["filename"].toString());
	EP_ADD_PARAM(arg,"filepath",evMap["filepath"].toString());
	EP_ADD_PARAM(arg,"filesize",evMap["filesize"].toString());
	EP_ADD_PARAM(arg,"channelnum",evMap["channelnum"].toString());
	EP_ADD_PARAM(arg,"start",startTime.toString("yyyy-MM-dd hh:mm:ss"));
	/*EP_ADD_PARAM(arg,"end",stopTime.toString("yyyy-MM-dd hh:mm:ss"));*/
	QString fileinfo;
	QVariantMap::const_iterator it;
	fileinfo.append("{");
	for(it=arg.begin();it!=arg.end();it++){
		/*fileinfo.append(it.key()).append(":").append(it.value().toString()).append(";");*/
		fileinfo.append("\\").append("\"").append(it.key()).append("\\").append("\"").append(":").append("\\").append("\"").append(it.value().toString()).append("\\").append("\"").append(",");
	}
	fileinfo.append("\\").append("\"").append("end").append("\\").append("\"").append(":").append("\\").append("\"").append(stopTime.toString("yyyy-MM-dd hh:mm:ss")).append("\\").append("\"").append(",");
	fileinfo.append("}");
	fileinfo.replace(",}","}");
	QString key;
	key="index_";
	fileMap.insert(key.append(fileKey),fileinfo);
	int ifileKey=fileKey.toInt();
	fileKey=QString::number(ifileKey+1);
	/*EventProcCall("GetRecordFile",arg);*/
}
void RecordPlayer::transSearchStop(QVariantMap &evMap)
{
	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"stopevent",evMap["stopevent"].toString());

	EventProcCall("SearchStop",arg);
}

void RecordPlayer::showEvent( QShowEvent * )
{
// 	if (NULL != m_pLocalPlayer)
// 	{
// 		m_pLocalPlayer->GroupSetVolume(0xAECBCA, &m_subRecPlayerView[m_currentWindID]);
// 	}
// 	m_subRecPlayerView[0].AudioEnabled(m_bIsOpenAudio);
	if(m_CurStatus!=STATUS_STOP){
		if (DevIsExit(m_devicename))
		{
			if (m_bIsHide)
			{
				m_bIsHide=false;
				RecordPlayerView::showSusWnd(true);
				GroupContinue();
			}else{
				//do nothing
			}
		}else{
			RecordPlayerView::showSusWnd(true);
			GroupContinue();
// 			GroupStop();
		}
	}

	// 加载语言
	loadlanguage();
}

void RecordPlayer::hideEvent( QHideEvent * )
{
// 	m_subRecPlayerView[0].AudioEnabled(false);
	if (m_CurStatus<STATUS_PAUSE)
	{
		RecordPlayerView::showSusWnd(false);
		GroupPause();
		m_bIsHide=true;
	}

}


bool RecordPlayer::DevIsExit( QString devicename)
{
	bool flags=false;
	IDeviceManager *pDeviceManager=NULL;
	IAreaManager *pAreaManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&pDeviceManager);
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void**)&pAreaManager);
	if (pAreaManager==NULL||pDeviceManager==NULL)
	{
		if (pDeviceManager!=NULL)
		{
			pDeviceManager->Release();
		}
		if (pAreaManager!=NULL)
		{
			pAreaManager->Release();
		}
		return false;
	}
	QStringList AreaListID=pAreaManager->GetAreaList();
	for (int i=0;i<AreaListID.size();++i)
	{
		QStringList DeviceListID=pDeviceManager->GetDeviceList(AreaListID.at(i).toInt());
		QStringList::const_iterator ite;
		for(ite=DeviceListID.constBegin();ite!=DeviceListID.constBegin();++ite){
			QVariantMap DeviceMap=pDeviceManager->GetDeviceInfo(ite->toInt());
			if (DeviceMap.value("name").toString()==devicename)
			{
				pAreaManager->Release();
				pDeviceManager->Release();
				return true;
			}
		}
	}
	//root area
	QStringList DeviceListID=pDeviceManager->GetDeviceList(0);
	for(int i=0;i<DeviceListID.size();++i){
		QVariantMap DeviceMap=pDeviceManager->GetDeviceInfo(DeviceListID.at(i).toInt());
		if (DeviceMap.value("name").toString()==devicename)
		{
			pAreaManager->Release();
			pDeviceManager->Release();
			return true;
		}
	}
	pAreaManager->Release();
	pDeviceManager->Release();
	return false;
}

int RecordPlayer::GetCurrentState()
{
	return m_CurStatus;
}

// int RecordPlayer::searchVideoFileEx( const QString &sDevName, const QString& sDate, const int& nTypes )
// {
// 	//input parameter error
// 	if (sDevName.isEmpty() || sDate.isEmpty() || nTypes <= 0 || nTypes > 15)
// 	{
// 		return 1;
// 	}
// 	if (NULL == m_pLocalRecordSearch)
// 	{
// 		return 1;
// 	}
// 	fileMap.clear();
// 	fileKey="0";
// 
// 	//get query interface
// 	ILocalRecordSearchEx *pRecordSearchEx = NULL;
// 	m_pLocalRecordSearch->QueryInterface(IID_ILocalRecordSearchEx, (void**)&pRecordSearchEx);
// 	if (NULL == pRecordSearchEx)
// 	{
// 		return 1;
// 	}
// 	int ret = pRecordSearchEx->searchVideoFileEx(sDevName, sDate, nTypes);
// 	if (ILocalRecordSearchEx::OK != ret)
// 	{
// 		return 1;//call function error
// 	}
// 	pRecordSearchEx->Release();
// 
// 	EventProcCall("GetRecordFile",fileMap);
// 	return 0;
// }

int RecordPlayer::searchVideoFileEx2( const int & nWndId, const QString & sDate, const QString & sStartTime, const QString & sEndTime, const int & nTypes )
{
// 	qDebug()<<(int)this<<" searchVideoFileEx2"<<nWndId<<sDate<<sStartTime<<sEndTime<<nTypes;
	
	if (nWndId < 0 || sDate.isEmpty() || sStartTime.isEmpty() || sEndTime.isEmpty() || nTypes < 0 || nTypes > 15)
	{
		return 1;
	}
	if (NULL == m_pLocalRecordSearch)
	{
		return 1;
	}
// 	fileMap.clear();
// 	fileKey = "0";
// 
// 	//get query interface
// 	ILocalRecordSearchEx *pRecSchEx = NULL;
// 	m_pLocalRecordSearch->QueryInterface(IID_ILocalRecordSearchEx, (void**)&pRecSchEx);
// 	if (NULL == pRecSchEx)
// 	{
// 		return 1;
// 	}
// 	int ret = pRecSchEx->searchVideoFileEx(nWndId, sDate, sStartTime, sEndTime, nTypes);
// 	if (ILocalRecordSearchEx::OK != ret)
// 	{
// 		return 1;//call function error
// 	}
// 	pRecSchEx->Release();
// 
// 	EventProcCall("GetRecordFileEx",fileMap);

	if (!m_schEvMap.contains(nWndId))
	{
		SearchProcess *pSchProc = new SearchProcess();
		m_schEvMap.insert(nWndId, pSchProc);
		connect(pSchProc, SIGNAL(sigSchRet(int, QVariantMap)), this, SLOT(sndToUI(int, QVariantMap)));
		pSchProc->setContext(m_pLocalRecordSearch);
		pSchProc->setPara(nWndId, sDate, sStartTime, sEndTime, nTypes);
		pSchProc->start();
		m_wndCount++;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"wnd lose:"<<nWndId;
		abort();
	}


	return 0;
}

void RecordPlayer::transRecordFilesEx( QVariantMap &evMap )
{
	QString fileinfo;
	QStringList infoList;
	QVariantMap::const_iterator it;
	for(it=evMap.begin();it!=evMap.end();it++)
	{
		infoList<<"\\\"" + it.key() + "\\\":\\\"" + it.value().toString() + "\\\"";
	}
	fileinfo = "{" + infoList.join(",") + "}";
	QString key;
	key="index_";
	fileMap.insert(key.append(fileKey),fileinfo);
	int ifileKey=fileKey.toInt();
	fileKey=QString::number(ifileKey+1);
}

int RecordPlayer::AddFileIntoPlayGroupEx( const int & nWndId,const QString& sDate,const QString & sStartTime,const QString & sEndTime,const int & nTypes )
{
	qWarning()<<(int)this<<" AddFileIntoPlayGroupEx"<<nWndId<<sDate<<sStartTime<<sEndTime<<nTypes;
	if (nWndId < 0|| sDate.isEmpty() || sStartTime.isEmpty() || sEndTime.isEmpty() || nTypes < 0 || nTypes > 15)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<" input error!";
		return 1;//input parameter error
	}
	QDate date = QDate::fromString(sDate, "yyyy-MM-dd");
	QTime start = QTime::fromString(sStartTime, "hh:mm:ss");
	QTime end = QTime::fromString(sEndTime, "hh:mm:ss");
	if (!date.isValid() || !start.isValid() || !end.isValid() || end <= start)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<" time is valid";
		return 1;//input parameter error
	}
	if (NULL == m_pLocalPlayer)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<" m_pLocalPlayer == NULL";
		return 1;
	}
	
	ILocalPlayerEx *pLocalPlayerEx = NULL;
	m_pLocalPlayer->QueryInterface(IID_ILocalPlayerEx, (void**)&pLocalPlayerEx);
	if (NULL == pLocalPlayerEx)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<" pLocalPlayerEx == NULL";
		return 1;
	}

	m_subRecPlayerView[m_wndNum%ARRAY_SIZE(m_subRecPlayerView)].setPlayingFlag(true);
// 	qDebug()<<(int)this<<" AddFileIntoPlayGroupEx m_wndNum:"<<m_wndNum;
	int nRet = pLocalPlayerEx->AddFileIntoPlayGroupEx(nWndId, &m_subRecPlayerView[m_wndNum%ARRAY_SIZE(m_subRecPlayerView)], date, start, end, nTypes);
	pLocalPlayerEx->Release();
	m_wndNum++;

	return nRet;
}

SearchProcess * RecordPlayer::getCurProc( int wndId )
{
	return m_schEvMap.value(wndId);;
}

void RecordPlayer::sndToUI( int wnd, QVariantMap evMap )
{
	SearchProcess *pSch = m_schEvMap.value(wnd);
	pSch->deleteLater();
	m_schEvMap.remove(wnd);
	EventProcCall("GetRecordFileEx", evMap);

	if (m_wndCount < MAX_WINDOWS_NUM && m_schEvMap.isEmpty())
	{
		QVariantMap item;
		item.insert("searchResult", "INCOMPLETE");
		EventProcCall("SearchRecordOver", item);
	}

	if (m_wndCount >= MAX_WINDOWS_NUM && m_schEvMap.isEmpty())
	{
		QVariantMap item;
		item.insert("searchResult", "SUCCESS");
		EventProcCall("SearchRecordOver", item);
		m_wndCount = 0;
	}
}

void RecordPlayer::throwException( QVariantMap &evMap )
{
	RecordPlayerView *pWnd = (RecordPlayerView*)evMap.take("pWnd").toUInt();
	evMap.insert("wndId", pWnd - m_subRecPlayerView);
	EventProcCall(QString("ThrowException"), evMap);
}

void RecordPlayer::loadlanguage()
{
	// Get language description from system
	QString sLang;
	ILocalSetting * pi;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void **)&pi);
	if (NULL != pi)
	{
		sLang = pi->getLanguage();
		pi->Release();
	}
	else
	{
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
	int i;
	for (i = 0; i < itemList.count(); i++)
	{
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

void RecordPlayer::slValidateFail( QVariantMap vmap )
{
	EventProcCall(QString("Validation"), vmap);
}

void RecordPlayer::screenShot( QString sUser,int nType )
{
	if (NULL!=m_pScreenShotDevice)
	{
		m_pScreenShotDevice->screenShot(sUser,nType,m_currentWindID);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as m_pScreenShotDevice is null";
	}
}

void RecordPlayer::transScreenShot( QVariantMap &evMap )
{
	EventProcCall("screenShot",evMap);
}
