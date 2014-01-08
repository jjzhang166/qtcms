#include "RecordPlayer.h"
#include "libpcom.h"
#include "qwfw.h"
#include <guid.h>
#include "IEventRegister.h"

RecordPlayer::RecordPlayer():
QWebPluginFWBase(this),
m_pLocakRecordSearch(NULL)
{
	//申请ILocalRecordSearch接口
	pcomCreateInstance(CLSID_LocalPlayer,NULL,IID_ILocalRecordSearch,(void **)&m_pLocakRecordSearch);

	//注册事件
	cbInit();
}

RecordPlayer::~RecordPlayer()
{
	if (NULL != m_pLocakRecordSearch)
	{
		m_pLocakRecordSearch->Release();
		m_pLocakRecordSearch = NULL;
	}
}

int RecordPlayer::cbInit()
{
	IEventRegister *pEvRegister = NULL;
	if (NULL == m_pLocakRecordSearch)
	{
		return 1;
	}

	m_pLocakRecordSearch->QueryInterface(IID_IEventRegister, (void**)&pEvRegister);
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

int RecordPlayer::searchDateByDeviceName(const QString& sdevname)
{
	if (sdevname.isEmpty())
	{
		return 1;
	}

	if (NULL == m_pLocakRecordSearch)
	{
		return 1;
	}

	int nRet = m_pLocakRecordSearch->searchDateByDeviceName(sdevname);
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
	if (sdevname.isEmpty() || !checkChannel(schannellist))
	{
		return 1;
	}
	QDateTime date = QDateTime::fromString(sdate,"yyyy-MM-dd");
	QTime timeStart = QTime::fromString(sbegintime,"hh:mm:ss");
	QTime timeEnd = QTime::fromString(sendtime,"hh:mm:ss");

	if (!date.isValid() || !timeStart.isValid() || !timeEnd.isValid())
	{
		return 1;
	}

	if (NULL == m_pLocakRecordSearch)
	{
		return 1;
	}

	int nRet = m_pLocakRecordSearch->searchVideoFile(sdevname, sdate, sbegintime, sendtime, schannellist);
	if (ILocalRecordSearch::OK != nRet)
	{
		return 1;
	}

	return 0;
}

bool RecordPlayer::checkChannel(const QString& schannellist)
{
	if (schannellist.isEmpty())
	{
		return false;
	}

	QStringList sltChannels = schannellist.split(";");
	QRegExp regTimeFormat("[0-9]{1,2};");
	int num = 0;
	QString temp = sltChannels[num] + ";";
	while(temp.contains(regTimeFormat))
	{
		num++;
		temp = sltChannels[num] + ";";
	}

	if (num == sltChannels.size() - 1)
	{
		return true;
	}
	else
	{
		return false;
	}
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
