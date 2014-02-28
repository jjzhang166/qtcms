#include "RecordPlayer.h"
#include "libpcom.h"
#include "qwfw.h"
#include <QtGui/QResizeEvent>
#include <qwfw_tools.h>
#include <guid.h>
#include "IEventRegister.h"

RecordPlayer::RecordPlayer():
QWebPluginFWBase(this),
m_pLocalRecordSearch(NULL),
m_pLocalPlayer(NULL),
m_pWindowDivMode(NULL),
m_currentWindID(0)
{
	//申请ILocalRecordSearch接口
	pcomCreateInstance(CLSID_LocalPlayer,NULL,IID_ILocalRecordSearch,(void **)&m_pLocalRecordSearch);
	//申请ILocalPlayer接口
	pcomCreateInstance(CLSID_LocalPlayer,NULL,IID_ILocalPlayer,(void **)&m_pLocalPlayer);
	//申请IWindowDivMode接口
	pcomCreateInstance(CLSID_DivMode2_2,NULL,IID_IWindowDivMode,(void **)&m_pWindowDivMode);

	for (int i = 0; i < ARRAY_SIZE(m_subRecPlayerView); i++)
	{
		m_subRecPlayerView[i].setParent(this);
		connect(&m_subRecPlayerView[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_subRecPlayerView[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));
		m_lstRecordPlayerWndList.insert(i,&m_subRecPlayerView[i]);
	}

	if (m_pWindowDivMode != NULL)
	{
		m_pWindowDivMode->setParentWindow(this);
		m_pWindowDivMode->setSubWindows(m_lstRecordPlayerWndList,ARRAY_SIZE(m_subRecPlayerView));
		m_pWindowDivMode->flush();
	}

	//注册事件
	cbInit();
}

RecordPlayer::~RecordPlayer()
{
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
			break;
		}
	}
	m_currentWindID = j;
}

int RecordPlayer::GetCurrentWnd()
{
	return m_currentWindID;
}

int RecordPlayer::searchDateByDeviceName(const QString& sdevname)
{
	qDebug()<<"RecordPlayer :searchDateByDeviceName:"<<sdevname;
	if (sdevname.isEmpty())
	{
		return 1;
	}

	if (NULL == m_pLocalRecordSearch)
	{
		return 1;
	}

	int nRet = m_pLocalRecordSearch->searchDateByDeviceName(sdevname);
	if (ILocalRecordSearch::OK != nRet)
	{
		return 1;
	}

	return 0;
}
int RecordPlayer::searchVideoFile(const QString& sdevname,
	const QString& sdate,
	const QString& sbegintime,
	const QString& sendtime,
	const QString& schannellist)
{
	qDebug()<<"RecordPlayer :searchVideoFile:"<<sdevname<<sbegintime<<sendtime<<schannellist;
	if (NULL == m_pLocalRecordSearch)
	{
		return 1;
	}

	int nRet = m_pLocalRecordSearch->searchVideoFile(sdevname, sdate, sbegintime, sendtime, schannellist);
	if (ILocalRecordSearch::OK != nRet)
	{
		return 1;
	}

	return 0;
}

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

int RecordPlayer::AddFileIntoPlayGroup(const QString &filelist,const int &nWndID,const QString &startTime,const QString &endTime)
{
	qDebug()<<"RecordPlayer :AddFileIntoPlayGroup:"<<filelist<<nWndID<<startTime<<endTime;
	if (NULL == m_pLocalPlayer || filelist.isEmpty() || nWndID <0 || nWndID >= ARRAY_SIZE(m_subRecPlayerView))
	{
		return 1;
	}

	QStringList lstFileList = filelist.split(",", QString::SkipEmptyParts);
	if (-1 == sortFileList(lstFileList))
	{
		return 1;
	}

	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");

	int nRet = m_pLocalPlayer->AddFileIntoPlayGroup(lstFileList, &m_subRecPlayerView[nWndID], start, end);
	if (1 == nRet)
	{
		return 1;
	}

	return 0;
}

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

	return 0;
}
int RecordPlayer::GroupStop()
{
	qDebug()<<"RecordPlayer :GroupStop:";
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}

	int nRet = m_pLocalPlayer->GroupStop();
	if (1 == nRet)
	{
		return 1;
	}

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

	return 0;
}

QString RecordPlayer::GetNowPlayedTime()
{
	QString playedTime = "";
	if (NULL == m_pLocalPlayer)
	{
		return playedTime;
	}

	QDateTime ptime = m_pLocalPlayer->GetNowPlayedTime();
	playedTime = ptime.toString("yyyy-MM-dd hh:mm:ss");
	qDebug()<<playedTime;
	QDateTime pCurdate;
	pCurdate.setDate(QDate::currentDate());
	QString CurrentTime;
	CurrentTime=QString("%1").arg(ptime.toTime_t()-pCurdate.toTime_t());
	qDebug()<<CurrentTime;
	return CurrentTime;
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
	qDebug()<<evMap["filename"].toString();
	EP_ADD_PARAM(arg,"filepath",evMap["filepath"].toString());
	EP_ADD_PARAM(arg,"filesize",evMap["filesize"].toString());
	EP_ADD_PARAM(arg,"channelnum",evMap["channelnum"].toString());
	EP_ADD_PARAM(arg,"startTime",startTime.toString("yyyy-MM-dd hh:mm:ss"));
	EP_ADD_PARAM(arg,"stopTime",stopTime.toString("yyyy-MM-dd hh:mm:ss"));
	EventProcCall("GetRecordFile",arg);

}
void RecordPlayer::transSearchStop(QVariantMap &evMap)
{
	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"stopevent",evMap["stopevent"].toString());

	EventProcCall("SearchStop",arg);
}
