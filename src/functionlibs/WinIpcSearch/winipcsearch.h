#ifndef WINIPCSEARCH_H
#define WINIPCSEARCH_H

#include "winipcsearch_global.h"
#include <IDeviceSearch.h>
#include <IEventRegister.h>
#include <guid.h>
#include <QMutex>
#include "WinIpcSock.h"
#include <IDeviceNetModify.h>


class  WinIpcSearch:public IDeviceSearch,
	public IEventRegister,
	public IDeviceNetModify
{
public:
	WinIpcSearch();
	~WinIpcSearch();

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

	virtual int SetNetworkInfo(const QString &sDeviceID,
		const QString &sAddress,
		const QString &sMask,
		const QString &sGateway,
		const QString &sMac,
		const QString &sPort,
		const QString &sUsername,
		const QString &sPassword);
private:
	void eventProcCall(QString sEvent,QVariantMap param);

private:
	int             m_nRef;
	QMutex          m_csRef;
	QStringList     m_sEventList;
	volatile bool	m_bFlush;
	volatile bool	m_bStop;
	int				m_nTimeInterval;
	QMultiMap<QString,   EventCBInfo>m_mEventCBMap;

	WinIpcSock m_WinIpcSock;

};

#endif // WINIPCSEARCH_H
