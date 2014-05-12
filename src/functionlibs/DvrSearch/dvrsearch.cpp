#include "dvrsearch.h"
#include <QElapsedTimer>


DvrSearch::DvrSearch():
m_nRef(0),
m_nTimeInterval(10),
m_bFlush(false)
{
    m_sEventList.insert(0, QString("SearchDeviceSuccess"));
}

DvrSearch::~DvrSearch(){
	m_running=false;
	while(QThread::isRunning()){
		msleep(10);
	}
}

long __stdcall DvrSearch::QueryInterface( const IID & iid,void **ppv )
{
    if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IDeviceSearch == iid)
	{
		*ppv = static_cast<IDeviceSearch*>(this);
	}
    else if (IID_IEventRegister == iid)
    {
        *ppv = static_cast<IEventRegister*>(this);
    }
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall DvrSearch::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall DvrSearch::Release()
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

int DvrSearch::Start()
{
	if (!QThread::isRunning())
	{
		QThread::start();
	}
	m_running=true;
	return 0;
}
int DvrSearch::Stop(){
	m_running=false;
	return 0;
}

int DvrSearch::Flush(){
	m_bFlush=true;
	return 0;
}


void DvrSearch::run(){
    m_pUdpSocket = new QUdpSocket;
    m_pUdpSocket->bind(UDP_PORT, QUdpSocket::ShareAddress);
    QElapsedTimer timer;
    timer.start();
    qint64 nLen = m_pUdpSocket->writeDatagram(SENDBUFF,strlen(SENDBUFF),QHostAddress::Broadcast ,UDP_PORT);
    while (m_running)
	{
        if (timer.elapsed() > m_nTimeInterval*1000 || m_bFlush)
        {
            timer.start();
            m_pUdpSocket->writeDatagram(SENDBUFF,strlen(SENDBUFF),QHostAddress::Broadcast ,UDP_PORT);
            m_bFlush = false;      
        }  
        Recv(); 
        msleep(10);
	} 
	m_pUdpSocket->close();
	delete m_pUdpSocket;
}

int DvrSearch::setInterval(int nInterval)
{
    if(nInterval > 0 && nInterval < 100)
    {
        m_nTimeInterval = nInterval;
        return 0;
    }
    else
    {
        return -1;
    }
}

IEventRegister * DvrSearch::QueryEventRegister() 
{
    IEventRegister * ret ;
    QueryInterface(IID_IEventRegister,(void **)&ret);
    //ret->Release();
    return ret;  
}


QStringList DvrSearch::eventList()
{      
	return m_sEventList;
}

int DvrSearch::queryEvent(QString eventName, QStringList &eventParamList)
{
    if (!m_sEventList.contains(eventName))
    {
        return IEventRegister::E_EVENT_NOT_SUPPORT;
    }
	if ( "SearchDeviceSuccess" == eventName)
	{
        eventParamList<<"SearchVendor_ID"  <<"SearchDeviceName_ID"  <<"SearchDeviceId_ID"     <<"SearchDeviceModelId_ID"
                      <<"SearchSeeId_ID"   <<"SearchChannelCount_ID"<<"SearchIP_ID"           <<"SearchMask_ID"
                      <<"SearchMac_ID"     <<"SearchGateway_ID"     <<"SearchHttpport_ID"     <<"SearchMediaPort_ID";
	}
	return IEventRegister::OK;
}

int DvrSearch::registerEvent(QString eventName,EventCallBack eventCB,void *pUser) 
{
    if (!m_sEventList.contains(eventName))
    {
        return IEventRegister::E_EVENT_NOT_SUPPORT;
    }
    EventCBInfo procInfo;
    procInfo.evCBName = eventCB;
    procInfo.pUser = pUser;
    m_mEventCBMap.insert(eventName, procInfo);
    return IEventRegister::OK;
}

void DvrSearch::eventProcCall(QString sEvent,QVariantMap param)
{
    if (m_sEventList.contains(sEvent))
    {
        EventCBInfo eventDes = m_mEventCBMap.value(sEvent);
        if (NULL != eventDes.evCBName)
        {
            eventDes.evCBName(sEvent, param, eventDes.pUser);
        }
    }
}

void DvrSearch::Recv()
{
	while (m_pUdpSocket->hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize(m_pUdpSocket->pendingDatagramSize());
		m_pUdpSocket->readDatagram(datagram.data(),datagram.size());
        
		QString strTmp(datagram.data());
		strTmp = strTmp.toUpper();
        if ( datagram.contains("SEARCHDEV") )
        {
            continue;
        }
        
		QByteArray     StrToOther;
        QStringList    strListInfo;
		StrToOther = ParseSearch(strTmp, "JAIP");
        strListInfo.insert(0,QString(StrToOther.data()));

		StrToOther = ParseSearch(strTmp, "ID");
	    strListInfo.insert(1,QString(StrToOther.data()));

        StrToOther = ParseSearch(strTmp, "PORT");
        strListInfo.insert(2,QString(StrToOther.data()));

		StrToOther = ParseSearch(strTmp, "HTTP");
        strListInfo.insert(3,QString(StrToOther.data()));

		StrToOther = ParseSearch(strTmp, "CH");
        strListInfo.insert(4,QString(StrToOther.data()));

		StrToOther=ParseSearch(strTmp,"MODEL");
		strListInfo.insert(5,QString(StrToOther.data()));
		if (strListInfo.at(0)=="0")
		{
			continue;
		}
		if (strListInfo.at(5)=="0")
		{
			 m_mEventCBParam.insert("SearchVendor_ID"        ,QVariant("DVR"));
		}else{
			m_mEventCBParam.insert("SearchVendor_ID"        ,QVariant(strListInfo.at(5)));
		}
       
        m_mEventCBParam.insert("SearchDeviceName_ID"    ,QVariant(""));
        m_mEventCBParam.insert("SearchDeviceId_ID"      ,QVariant(""));
        m_mEventCBParam.insert("SearchDeviceModelId_ID" ,QVariant(""));
        m_mEventCBParam.insert("SearchSeeId_ID"         ,QVariant(strListInfo.at(1)));
        m_mEventCBParam.insert("SearchChannelCount_ID"  ,QVariant(strListInfo.at(4)));
        m_mEventCBParam.insert("SearchIP_ID"            ,QVariant(strListInfo.at(0)));
        m_mEventCBParam.insert("SearchMask_ID"          ,QVariant(""));
        m_mEventCBParam.insert("SearchMac_ID"           ,QVariant(""));
        m_mEventCBParam.insert("SearchGateway_ID"       ,QVariant(""));     
		m_mEventCBParam.insert("SearchHttpport_ID"      ,QVariant(strListInfo.at(3)));
		m_mEventCBParam.insert("SearchMediaPort_ID"     ,QVariant(strListInfo.at(2)));
        eventProcCall(QString("SearchDeviceSuccess"),m_mEventCBParam);

	} // end of while
}

QByteArray DvrSearch::ParseSearch(const QString &content, const QString &index)
{
	int nTmp= 0;
    nTmp = content.indexOf(index,0);
    if (-1 == nTmp)
    {
        return "0"; //no this item
    }
	QString strTmp(content.mid(nTmp+ index.length()));
	QByteArray QBRet( strTmp.toLatin1());

	nTmp = QBRet.indexOf('&',0);
	if (-1 != nTmp)
	{
		QBRet.truncate(nTmp);
	}
    if (QBRet.isEmpty()) //this item has no value
    {
        QBRet.append("");
    }
	return QBRet;
}