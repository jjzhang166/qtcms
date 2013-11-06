#include "HiChipSearch.h"
#include <guid.h>
#include <QtNetwork/QHostAddress>
#include <QDateTime>
#include <QDebug>



HiChipSearch::HiChipSearch() :
m_nRef(0),
m_nInterval(5)
{
	m_bRunning = false;
	m_bFlush = false;

	Socket = new QUdpSocket(this);

}

HiChipSearch::~HiChipSearch()
{
	if (m_bRunning)
	{
		Stop();
	}
	m_bEnd = true;
	delete Socket;
}

int HiChipSearch::Start()
{
	bool Ret = false;
	Ret = Socket->bind(MCASTPORT, QUdpSocket::ShareAddress);
	Ret &= Socket->joinMulticastGroup(QHostAddress(MCASTADDR));
	if (!Ret)
	{
		return -1;
	}

	if (m_bRunning)
	{
		return 0;
	}

	m_bRunning = true;
	m_bEnd = false;
	QThread::start();
	
	return 0;
}

void HiChipSearch::run()
{
	QByteArray arr;
	arr += "SEARCH * HDS/1.0\r\n";
	arr += "CSeq:1\r\n";
	arr += "Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC\r\n";
	arr += "Accept-Type:text/HDP\r\n";
	arr += "Content-Length:0\r\n";
	arr += "\r\n";

	QDateTime timeStart =  QDateTime::currentDateTime();

	while(!m_bEnd)
	{
		QDateTime timeEnd = QDateTime::currentDateTime();
		if (timeEnd.toTime_t() - timeStart.toTime_t() >= m_nInterval || m_bFlush)
		{
			Socket->writeDatagram(arr,QHostAddress(QString(MCASTADDR)), MCASTPORT);
			timeStart = QDateTime::currentDateTime();
			m_bFlush = false;
		}

		sleep(1);
		Receive();
	}
}

void HiChipSearch::Receive()
{
	QByteArray datagrm;

	while(Socket->hasPendingDatagrams())
	{
		datagrm.resize(Socket->pendingDatagramSize());
		Socket->readDatagram(datagrm.data(), datagrm.size());
		if (datagrm.contains("HDS/1.0 200 OK") && datagrm.contains("Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC"))
		{
			QVariantMap item;
			parseSearchAck(datagrm,item);
			QString evName = QString("DeviceFound");
			if (eventMap.find(evName) != eventMap.end())
			{
				IPCSearchCB proc = eventMap.value(evName).proc;
				if ( NULL != proc )
				{
					proc(evName,item, eventMap.value(evName).puser);
				}
			}
		}
	}

	m_bRunning = false;
}


void HiChipSearch::getItem(QByteArray buff, QByteArray source, QString& dest)
{
	dest.clear();
	if (buff.contains(source))
	{
		int length = buff.size() - buff.indexOf(source) - source.size();
		QByteArray result = buff.right(length);
		QString strTemp = result.left(result.indexOf("\n") - 1);
		if (strTemp.isEmpty())
		{
			dest = QString("-1");
		}
		else
		{
			dest = strTemp;
		}

	}
	else
	{
		dest = QString("-1");
	}
}

void HiChipSearch::parseSearchAck(QByteArray buff, QVariantMap& itemmap)
{
	QString context;

	getItem(buff,"Device-Name=",context);
	itemmap.insert("deviceName", context);

	getItem(buff,"Device-ID=",context);
	itemmap.insert("deviceID", context);

	getItem(buff,"Device-Model=",context);
	itemmap.insert("deviceModel", context);

	getItem(buff,"Esee-ID=",context);
	itemmap.insert("eseeID", context);

	getItem(buff,"Channel-Cnt=",context);
	itemmap.insert("channelCount", context);

	getItem(buff,"IP=",context);
	itemmap.insert("IP", context);

	getItem(buff,"MASK=",context);
	itemmap.insert("mask", context);

	getItem(buff,"MAC=",context);
	itemmap.insert("mac", context);

	getItem(buff,"Gateway=",context);
	itemmap.insert("Gateway", context);

	getItem(buff,"Http-Port=",context);
	itemmap.insert("httpPort", context);
}


int HiChipSearch::setInterval(int nInterval)
{
	m_nInterval = nInterval;

	return 0;
}

int HiChipSearch::Flush()
{
	m_bFlush = true;
	return 0;
}

int HiChipSearch::Stop()
{
	if (m_bEnd)
	{
		return -1;
	}
	while(m_bRunning)
	{
		sleep(1);
	}
	terminate();
	m_bEnd = true;

	return 0;
}

IEventRegister* HiChipSearch::QueryEventRegister()
{
	return static_cast<IEventRegister*>(this);
}

QStringList HiChipSearch::eventList()
{
	QStringList evname;
	QMultiMap<QString, ProcInfoItem_t>::iterator it;
	for (it = eventMap.begin(); it != eventMap.end(); it++)
	{
		evname<<it.key();
	}

	return evname;
}

int HiChipSearch::queryEvent(QString eventName,QStringList& eventParams)
{
	QMultiMap<QString, ProcInfoItem_t>::iterator it;
	for (it = eventMap.begin(); it != eventMap.end(); it++)
	{
		if (it.key() == eventName)
		{
			eventParams<<"IP"<<"deviceID"<<"eseeID"<<"deviceName"<<"deviceModel"<<"channelCount"<<"mask"<<"Gateway"<<"mac"<<"httpPort";
		}
	}

	if (it == eventMap.end())
	{
		return IEventRegister::E_INVALID_PARAM;
	}

	return IEventRegister::OK;
}

int HiChipSearch::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	if (eventMap.find(eventName) == eventMap.end())
	{
		ProcInfoItem_t procInfo;
		procInfo.proc = proc;
		procInfo.puser = pUser;
		eventMap.insert(eventName,procInfo);
		return IEventRegister::OK;
	}
	else
	{
		return IEventRegister::E_INVALID_PARAM;
	}
}


long __stdcall HiChipSearch::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IDeviceSearch == iid)
	{
		*ppv = static_cast<IDeviceSearch *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall HiChipSearch::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall HiChipSearch::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

