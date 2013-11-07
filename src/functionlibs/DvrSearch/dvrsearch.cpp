#include "dvrsearch.h"
#include <QElapsedTimer>


DvrSearch::DvrSearch():
m_nRef(0),
m_nTimeInterval(5),
m_bFlush(false),
m_bStop(false),
m_bStart(false),
m_bIsRunning(false),
m_eventCB(NULL),
m_pEventCBParam(NULL)
{
    m_pUdpSocket = new QUdpSocket(this);
    m_pUdpSocket->bind(UDP_PORT, QUdpSocket::ShareAddress);
}

DvrSearch::~DvrSearch()
{ 
     if (m_pUdpSocket != NULL )
     {
         delete m_pUdpSocket;
         m_pUdpSocket = NULL;
     }
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
    if (m_bStart )
    {
        return -1;
    }  
    m_bStart     = true;
    m_bStop      = false;

    start();
   
    return 0;
}

int DvrSearch::Stop()
{
    if (!m_bStart) // start not yet
    {
        return -1;
    }
    m_bStop      = true;
    m_bStart     = false;
    while (m_bIsRunning)
    {
        wait();
    }
	return 0;
}

int DvrSearch::Flush()
{
	m_bFlush = true;
    m_bStop  = false;
	return 0;
}


void DvrSearch::run()
{
    QElapsedTimer timer;
    timer.start();
    m_pUdpSocket->writeDatagram(g_cSendBuff,sizeof(g_cSendBuff),QHostAddress::Broadcast ,UDP_PORT);
    while (!m_bStop)
	{
        m_bIsRunning = true;
        if (timer.elapsed() > m_nTimeInterval*1000 || m_bFlush)
        {
            timer.start();
            m_pUdpSocket->writeDatagram(g_cSendBuff,sizeof(g_cSendBuff),QHostAddress::Broadcast ,UDP_PORT);
            m_bFlush = false;      
        } 
        if (m_sEventCBParam == "deviceFound")
        {  
            Recv(); 
            msleep(10);
        } 
	} 
    m_bIsRunning = false;
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
	m_sEventList = EventCBMap.keys();
	return m_sEventList;
}

int DvrSearch::queryEvent(QString eventName, QStringList &eventParamList)
{
	if ( "deviceFound" == eventName)
	{
		eventParamList<<"IP"<<"ID"<<"Http"<<"Media"<<"ChannelCount";
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
    QMap<QString,   EventCBInfo>::const_iterator it = EventCBMap.find(eventName);
    while(  it != EventCBMap.end()&& it.key() == eventName)
    {
        if (it.value().evCBName == eventCB && it.value().pEventCBP == pUser)
        {
            nRet = -1;
            break;
        }
        ++it;
    }
    if (nRet != -1) 
    {
        EventCBInfo evCBInfoTmp;
        evCBInfoTmp.evCBName  = eventCB;
        evCBInfoTmp.pEventCBP = pUser;
        EventCBMap.insertMulti(eventName, evCBInfoTmp);	
    }
	m_sEventCBParam = eventName;
	m_eventCB       = eventCB;
	m_pEventCBParam = pUser;
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
    
        if ( datagram.contains("SEARCHDEV") )
        {
            continue;
        }
        printf("%s\n",datagram.data());
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
        
        m_mEventCBParam.insert("IP"            ,QVariant(strListInfo.at(0)));
        m_mEventCBParam.insert("ID"            ,QVariant(strListInfo.at(1)));
		m_mEventCBParam.insert("Http"          ,QVariant(strListInfo.at(3)));
		m_mEventCBParam.insert("Media"         ,QVariant(strListInfo.at(2)));
		m_mEventCBParam.insert("ChannelCount"  ,QVariant(strListInfo.at(4)));
 		
		if (m_eventCB != NULL)
		{
			nRet = m_eventCB(m_sEventCBParam, m_mEventCBParam, m_pEventCBParam);
		}
	} // end of while
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
        QBRet.append("-1");
    }
	return QBRet;
}