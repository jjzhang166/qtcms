#include "dvrsearch.h"



DvrSearch::DvrSearch():
m_nRef(0),
m_nTimeInterval(1),
m_bFlush(false),
m_bStop(false),
m_bStart(false),
m_eventCB(NULL),
m_pEventCBParam(NULL)
{
    m_pUdpSocket = new QUdpSocket(this);
    m_pUdpSocket->bind(UDP_PORT, QUdpSocket::ShareAddress);
}

DvrSearch::~DvrSearch()
{
    while(this->isRunning())
    {
        this->exit();
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
    if (m_bStart )
    {
        return -1;
    }
    m_csRef.lock();
        m_bStart     = true;
        m_bStop      = false;
    m_csRef.unlock();
    start();
    
    return 0;
}

int DvrSearch::Stop()
{
    if (m_bStop)
    {
        return -1;
    }
    m_csRef.lock();
        m_bStop      = true;
        m_bStart     = false;
    m_csRef.unlock();
	return 0;
}

int DvrSearch::Flush()
{
	m_csRef.lock();
	    m_bFlush = true;
        m_bStop  = false;
	m_csRef.unlock();
	return 0;
}


void DvrSearch::run()
{
    while (!m_bStop || m_bFlush )
	{
        m_pUdpSocket->writeDatagram(g_cSendBuff,sizeof(g_cSendBuff),QHostAddress::Broadcast ,UDP_PORT);
        sleep(1);
        if (m_sEventCBParam == "deviceFound")
        {  
            Recv();  
        }
               
        if (!m_bFlush)
        {
            sleep(m_nTimeInterval); 
        }  
        m_csRef.lock();
            m_bFlush = false;
        m_csRef.unlock();
	}        
}

int DvrSearch::setInterval(int nInterval)
{
    if(nInterval > 0 && nInterval < 100)
    {
        m_csRef.lock();
        m_nTimeInterval = nInterval;
        m_csRef.unlock();
        return 0;
    }
    else
    {
        return -1;
    }
}

IEventRegister * DvrSearch::QueryEventRegister() 
{
	return static_cast<IEventRegister*>(this);
}


QStringList DvrSearch::eventList()
{
    m_csRef.lock();
	m_sEventList = EventCBMap.keys();
    m_csRef.unlock();
	return m_sEventList;
}

int DvrSearch::queryEvent(QString eventName, QStringList &eventParamList)
{
	if ( "deviceFound" == eventName)
	{
		eventParamList<<"JAIP"<<"ID"<<"HTTP"<<"PORT"<<"CH";
		return 0;
	}
    else
	{
		return -1;
	}
}

int DvrSearch::registerEvent(QString eventName,EventCallBack eventCB,void *pUser) 
{
    int nRet = -2;
    QMap<QString,   EventCallBack>::const_iterator it = EventCBMap.find(eventName);
    while(  it != EventCBMap.end()&& it.key() == eventName)
    {
        if (it.value() == eventCB)
        {
            nRet = -1;
            break;
        }
        else
        {
            nRet = 0;
        }
        ++it;
    }
    if (nRet != -1)
    {
        m_csRef.lock();
        EventCBMap.insertMulti(eventName, eventCB);	
        m_csRef.unlock();
    }
    m_csRef.lock();
	m_sEventCBParam = eventName;
	m_eventCB       = eventCB;
	m_pEventCBParam = pUser;
    m_csRef.unlock();
    return nRet;
}

int DvrSearch::Recv()
{
	int nRet = 0;
    
	while (m_pUdpSocket->hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize(m_pUdpSocket->pendingDatagramSize());
		m_pUdpSocket->readDatagram(datagram.data(),datagram.size());
        
		QString strTmp(datagram.data());
		strTmp = strTmp.toUpper();

        bool bStrExist = true;
        bool bTmp  = strTmp.contains("JAIP");
        bStrExist &=bTmp;
        bTmp =  strTmp.contains("ID");
        bStrExist &=bTmp;
        bTmp =  strTmp.contains("PORT");
        bStrExist &=bTmp;
        bTmp =  strTmp.contains("HTTP");
        bStrExist &=bTmp;
        bTmp =  strTmp.contains("CH");
        bStrExist &=bTmp;
        if (! bStrExist || datagram.contains("SEARCHDEV") )
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
        
        m_csRef.lock();
		m_mEventCBParam.insert("JAIP  "    ,QVariant(strListInfo.at(0)));
		m_mEventCBParam.insert("ID    "    ,QVariant(strListInfo.at(1)));
		m_mEventCBParam.insert("HTTP  "    ,QVariant(strListInfo.at(3)));
		m_mEventCBParam.insert("PORT  "    ,QVariant(strListInfo.at(2)));
		m_mEventCBParam.insert("CH    "    ,QVariant(strListInfo.at(4)));
        m_csRef.unlock();
		
		if (m_eventCB != NULL)
		{
			nRet = m_eventCB(m_sEventCBParam, m_mEventCBParam, m_pEventCBParam);
		}
	}
	if (nRet < 0 )
	{
		return nRet;
	}
	else 
    {      
		return 0;
    }
}

QByteArray DvrSearch::ParseSearch(const QString &content, const QString &index)
{
	int nTmp= 0;
    nTmp = content.indexOf(index,0);
	QString strTmp(content.mid(nTmp+ index.length()));
	QByteArray QBRet( strTmp.toLatin1());

	nTmp = QBRet.indexOf('&',0);
	if (-1 != nTmp)
	{
		QBRet.truncate(nTmp);
	}
    if (QBRet.isEmpty())
    {
        QBRet.append("-1");
    }
	return QBRet;
}