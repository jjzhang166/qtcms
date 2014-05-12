#include "HiChipSearch.h"
#include <guid.h>
#include <QtNetwork/QHostAddress>
#include <QDateTime>
#include <QDebug>
#include <QtNetwork>

HiChipSearch::HiChipSearch() :
m_nRef(0),
m_nInterval(10000),
m_running(false)
{
	m_bReceiving = false;
	m_bFlush = false;
	m_eventList << "SearchDeviceSuccess"<<"SettingStatus";

}

HiChipSearch::~HiChipSearch()
{
	m_running=false;
	while(QThread::isRunning()){
		msleep(10);
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
int HiChipSearch::Start(){
	if (!QThread::isRunning())
	{
		QThread::start();
	}
	m_running=true;
	return 0;
}

void HiChipSearch::run()
{
	m_Socket = new QUdpSocket();
	QString address = GetHostAddress();
	if (address.isEmpty())
	{
		m_Socket->close();
		delete m_Socket;
		return;
	}
	bool Ret = m_Socket->bind(QHostAddress(address),MCASTPORT, QUdpSocket::ShareAddress);
	Ret &= m_Socket->joinMulticastGroup(QHostAddress(MCASTADDR));

	if (!Ret)
	{
		m_Socket->close();
		delete m_Socket;
		return;
	}
	//
	QByteArray arr;
	arr += "SEARCH * HDS/1.0\r\n";
	arr += "CSeq:1\r\n";
	arr += "Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC\r\n";
	arr += "Accept-Type:text/HDP\r\n";
	arr += "Content-Length:0\r\n";
	arr += "\r\n";

	QTime timeStart;
	timeStart.start();
	m_Socket->writeDatagram(arr,QHostAddress(QString(MCASTADDR)), MCASTPORT);

	while(m_running)
	{
		if (timeStart.elapsed() >= m_nInterval || m_bFlush)
		{
			qint64 bytes = m_Socket->writeDatagram(arr,QHostAddress(QString(MCASTADDR)), MCASTPORT);
			timeStart.start();
			m_bFlush = false;
		}
		Receive();
		msleep(500);
	}
	m_Socket->close();
	delete m_Socket;
}


void HiChipSearch::Receive()
{
	QByteArray datagrm;
	QVariantMap netItem;

	m_bReceiving = true;
	while(m_Socket->hasPendingDatagrams())
	{
		datagrm.resize(m_Socket->pendingDatagramSize());
		m_Socket->readDatagram(datagrm.data(), datagrm.size());
		if (datagrm.contains("HDS/1.0 200 OK") && datagrm.contains("nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC"))
		{
			QVariantMap item;
			parseSearchAck(datagrm,item);
			eventProcCall(QString("SearchDeviceSuccess"),item);
		}
		else if (datagrm.contains("MCTP/1.0 200 OK") && datagrm.contains("[Success] set net information OK!"))
		{
			netItem.insert("Status", "set info success");
		}
		else if (datagrm.contains("MCTP/1.0 200 OK") && datagrm.contains("[Success] set port !"))
		{
			netItem.insert("Status", "set port success");
		}
		else if (datagrm.contains("HDS/1.0 401 Unauthorized"))
		{
			netItem.insert("Status", "Unauthorized");
		}
		if (!netItem.isEmpty())
		{
			eventProcCall(QString("SettingStatus"),netItem);
			qDebug()<<netItem;
			qDebug()<<datagrm;
			netItem.clear();
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
	itemmap.insert("SearchDeviceId_ID", context);
// 	itemmap.insert("SearchDeviceId_ID", context);
//	itemmap.insert("SearchSeeId_ID", context);
	getItem(buff,"MASK=",context);
	itemmap.insert("SearchMask_ID", context);

	getItem(buff,"Device-Model=",context);
	itemmap.insert("SearchDeviceModelId_ID", context);

	getItem(buff,"Esee-ID=",context);
// 	itemmap.insert("SearchSeeId_ID", context);
	itemmap.insert("SearchSeeId_ID", context);
//	itemmap.insert("SearchDeviceId_ID", context);
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

	itemmap.insert("SearchMediaPort_ID", context);
	itemmap.insert("SearchVendor_ID", "IPC");
	

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

int HiChipSearch::Stop(){
	m_running=false;
	return 0;
}

int HiChipSearch::SetNetworkInfo(const QString &sDeviceID,
	const QString &sAddress,
	const QString &sMask,
	const QString &sGateway,
	const QString &sMac,
	const QString &sPort,
	const QString &sUsername,
	const QString &sPassword)
{
	if (sDeviceID.isEmpty())
	{
		return IDeviceNetModify::E_INVALID_PARAM;
	}

	QByteArray content;
	int nCSeq = 2;
	char buff[1024] = {0};
	m_netInfo.clear();
	if (sAddress.size() > 0 || sMask.size() > 0 || sGateway.size() > 0 || sMac.size() > 0)
	{
		content += "netconf set";
		if(sAddress.size() > 0)
		{
			content += " -ipaddr " + sAddress;
		}
		if (sMask.size() > 0)
		{
			content += " -netmask " + sMask;
		}
		if (sGateway.size() > 0)
		{
			content += " -gateway " + sGateway;
		}
		if(sMac.size() > 0)
		{
			content += " -hwaddr " + sMac;
		}
		content += "\r\n";
	}
	unsigned int uiPort = sPort.toUInt();
	if(!sPort.isEmpty() && uiPort >= 0 && uiPort < 65536)
	{
		content += "httpport set -httpport " + QString("%1").arg(uiPort) + "\r\n";
	}
	qsnprintf(buff, 1024, "CMD * HDS/1.0\r\n"
		"CSeq:%d\r\n"
		"Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC\r\n"
		"Accept-Type:text/HDP\r\n"
		"Authorization:Basic %s:%s\r\n"
		"Device-ID:%s\r\n"
		"Content-Length:%d\r\n"
		"\r\n"
		"%s", nCSeq, sUsername.toLatin1().data(), sPassword.toLatin1().data(), sDeviceID.toLatin1().data(), content.size(), content.data());
	m_netInfo.append(buff);
	if (m_netInfo.isEmpty())
	{
		return IDeviceNetModify::E_SYSTEM_FAILED;
	}
		qDebug()<<m_netInfo;
		if (m_Socket==NULL)
		{
			return IDeviceNetModify::E_SYSTEM_FAILED;
		}
		m_Socket->writeDatagram(m_netInfo, QHostAddress(QString(MCASTADDR)), MCASTPORT);
		
	nCSeq++;

	

	return IDeviceNetModify::OK;
}

IEventRegister* HiChipSearch::QueryEventRegister()
{
	IEventRegister * ret ;
	QueryInterface(IID_IEventRegister,(void **)&ret);
	return ret;
}

QStringList HiChipSearch::eventList()
{
	return m_eventList;
}

int HiChipSearch::queryEvent(QString eventName,QStringList& eventParams)
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}

	if ("SearchDeviceSuccess" == eventName)
	{
		eventParams<<"SearchVendor_ID"<<"SearchIP_ID"<<"SearchDeviceId_ID"<<"SearchSeeId_ID"<<"SearchDeviceName_ID"<<"SearchDeviceModelId_ID"<<"SearchChannelCount_ID"<<"SearchMask_ID"<<"SearchGateway_ID"<<"SearchMac_ID"<<"SearchHttpport_ID"<<"SearchMediaPort_ID";
	}
	if ("SettingStatus" == eventName)
	{
		eventParams<<"Status";
	}

	return IEventRegister::OK;

}

int HiChipSearch::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}

	ProcInfoItem_t procInfo;
	procInfo.proc = proc;
	procInfo.puser = pUser;
	m_eventMap.insert(eventName,procInfo);
	return IEventRegister::OK;
}


long __stdcall HiChipSearch::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IDeviceSearch == iid)
	{
		*ppv = static_cast<IDeviceSearch *>(this);
	}
	else if (IID_IDeviceNetModify == iid)
	{
		*ppv = static_cast<IDeviceNetModify *>(this);
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

void HiChipSearch::eventProcCall( QString sEvent,QVariantMap param )
{
	if (m_eventList.contains(sEvent))
	{
		ProcInfoItem_t eventDes = m_eventMap.value(sEvent);
		if (NULL != eventDes.proc)
		{
			eventDes.proc(sEvent,param,eventDes.puser);
		}
	}
}
