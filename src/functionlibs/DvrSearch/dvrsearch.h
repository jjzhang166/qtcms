#ifndef DVRSEARCH_H
#define DVRSEARCH_H

#include "dvrsearch_global.h"
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>
#include <QThread>
#include <QMutex>
#include <QStringList>
#include <iterator>
#include "IDeviceSearch.h"
#include "IEventRegister.h"
#include "qwfw.h"
#include <guid.h>


class DvrSearch :public QThread,
	public IDeviceSearch,
	public IEventRegister
{
public:
	DvrSearch();
	~DvrSearch();
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual int Start() ;
	virtual int Stop() ;
	virtual int Flush() ;
	virtual int setInterval(int nInterval) ;
	virtual IEventRegister * QueryEventRegister() ;

	virtual QStringList eventList() ;
	virtual int queryEvent(QString eventName, QStringList& eventParamList) ;
	virtual int registerEvent(QString eventName,EventCallBack eventCB,void *pUser) ;

protected:
	void run();

public:
	QByteArray ParseSearch(const QString &content, const QString &str);
	int  Recv( );

private:
	int             m_nRef;
	QMutex          m_csRef;
	QUdpSocket *    m_pUdpSocket;
	int             m_nTimeInterval;
	volatile  bool  m_bFlush;
    int             m_nStopped;// -1 before start 0 already start(not stopped)
    // 1 after stopped 2 flush 
    volatile  bool  m_bThreadRunning;
    QStringList     m_sEventList;

	QMap<QString,   EventCBInfo>EventCBMap;
	EventCallBack   m_eventCB;
	void*           m_pEventCBParam;
	QString         m_sEventCBParam;
	QVariantMap     m_mEventCBParam;   
};

#endif // DVRSEARCH_H
