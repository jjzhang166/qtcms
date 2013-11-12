#include "HiChipSearch.h"
#include <guid.h>
#include <QtNetwork/QHostAddress>
#include <QDateTime>
#include <QDebug>
#include <QtNetwork>

HiChipSearch::HiChipSearch() :
m_nRef(0),
m_nInterval(10000)
{
	m_bReceiving = false;
	m_bFlush = false;

	Socket = new QUdpSocket();
	

}

HiChipSearch::~HiChipSearch()
{
	if (Socket)
	{
		Socket->close();
		delete Socket;
	}
}

QString HiChipSearch::GetHostAddress()
{
	QString address;
	QList<QHostAddress> List = QNetworkInterface::allAddresses();
	QList<QHostAddress>::iterator it;
	for (it = List.begin(); it != List.end(); it++)
	{
		if (it->protocol() == QAbstractSocket::IPv4Protocol)
		{
			address = it->toString();
			break;
		}
	}
	return address;
}

int HiChipSearch::Start()
{
	if (!QThread::isRunning())
	{
		QString address = GetHostAddress();
		if (address.isEmpty())
		{
			return -1;
		}

// 		bool Ret = Socket->bind(MCASTPORT, QUdpSocket::ShareAddress);
		bool Ret = Socket->bind(QHostAddress(address),MCASTPORT, QUdpSocket::ShareAddress);
 		Ret &= Socket->joinMulticastGroup(QHostAddress(MCASTADDR));

		if (!Ret)
		{
			return -1;
		}

		m_bEnd = false;

		QThread::start();

		return 0;
	}

	return -1;

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

	QTime timeStart;
	timeStart.start();
	Socket->writeDatagram(arr,QHostAddress(QString(MCASTADDR)), MCASTPORT);

	while(!m_bEnd)
	{
		if (timeStart.elapsed() >= m_nInterval || m_bFlush)
		{
			qint64 bytes = Socket->writeDatagram(arr,QHostAddress(QString(MCASTADDR)), MCASTPORT);
			timeStart.start();
			m_bFlush = false;
		}
		Receive();
		msleep(50);
	}
}

void HiChipSearch::Receive()
{
	QByteArray datagrm;

	m_bReceiving = true;
	while(Socket->hasPendingDatagrams())
	{
		datagrm.resize(Socket->pendingDatagramSize());
		Socket->readDatagram(datagrm.data(), datagrm.size());
		if (datagrm.contains("HDS/1.0 200 OK") && datagrm.contains("nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC"))
		{
			QVariantMap item;
			parseSearchAck(datagrm,item);
			QString evName = QString("SearchDeviceSuccess");
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

	m_bReceiving = false;
}


void HiChipSearch::getItem(QByteArray buff, QByteArray source, QString& dest)
{
	dest.clear();
	if (buff.contains(source))
	{
		int length = buff.size() - buff.indexOf(source) - source.size();
		QByteArray result = buff.right(length);
		dest = result.left(result.indexOf("\n") - 1);
	}
}

void HiChipSearch::parseSearchAck(QByteArray buff, QVariantMap& itemmap)
{
	QString context;

	getItem(buff,"Device-Name=",context);
	itemmap.insert("SearchDeviceName_ID", context);

	getItem(buff,"Device-ID=",context);
// 	itemmap.insert("SearchDeviceId_ID", context);
	itemmap.insert("SearchSeeId_ID", context);

	getItem(buff,"Device-Model=",context);
	itemmap.insert("SearchDeviceModelId_ID", context);

	getItem(buff,"Esee-ID=",context);
// 	itemmap.insert("SearchSeeId_ID", context);
	itemmap.insert("SearchDeviceId_ID", context);

	getItem(buff,"Channel-Cnt=",context);
	itemmap.insert("SearchChannelCount_ID", context);

	getItem(buff,"IP=",context);
	itemmap.insert("SearchIP_ID", context);

	getItem(buff,"MASK=",context);
	itemmap.insert("SearchMask_ID", context);

	getItem(buff,"MAC=",context);
	itemmap.insert("SearchMac_ID", context);

	getItem(buff,"Gateway=",context);
	itemmap.insert("SearchGateway_ID", context);

	getItem(buff,"Http-Port=",context);
	itemmap.insert("SearchHttpport_ID", context);

	itemmap.insert("SearchMediaPort_ID", "");
	itemmap.insert("SearchVendor_ID", "JUAN IPC");
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
	while(m_bReceiving)
	{
		//wait for receive info
	}
	m_bEnd = true;

	Socket->close();

	wait();

	return 0;
}

IEventRegister* HiChipSearch::QueryEventRegister()
{
	IEventRegister * ret ;
	QueryInterface(IID_IEventRegister,(void **)&ret);
	return ret;
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
	if ("SearchDeviceSuccess" == eventName)
	{
		eventParams<<"SearchVendor_ID"<<"SearchIP_ID"<<"SearchDeviceId_ID"<<"SearchSeeId_ID"<<"SearchDeviceName_ID"<<"SearchDeviceModelId_ID"<<"SearchChannelCount_ID"<<"SearchMask_ID"<<"SearchGateway_ID"<<"SearchMac_ID"<<"SearchHttpport_ID"<<"SearchMediaPort_ID";
		return IEventRegister::OK;
	}
	else
	{
		return IEventRegister::E_INVALID_PARAM;
	}
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

