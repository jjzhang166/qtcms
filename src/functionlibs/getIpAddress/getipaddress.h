#ifndef GETIPADDRESS_H
#define GETIPADDRESS_H
#include "IEventRegister.h"
#include <guid.h>
#include "getipaddress_global.h"
#include <IGetIpAddress.h>
#include <QMutex>
#include "getIpWinSock.h"
typedef int (__cdecl *getIpAddressEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagGetIpAddressProInfo{
	getIpAddressEventCb proc;
	void *pUser;
}tagGetIpAddressProInfo;
class  getIpAddress:public IEventRegister,
	public IGetIpAddress
{
public:
	getIpAddress();
	~getIpAddress();

public:
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	virtual bool getIpAddressEx(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
private:
	bool getIpAddressInLan(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
	bool getIpAddressInWan(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
private:
	volatile int m_nRef;
	QMutex m_csRef;
	QStringList m_sEventList;
	QMultiMap<QString,tagGetIpAddressProInfo> m_tEventMap;
	getIpWinSock m_tGetIpWinSock;
};

#endif // GETIPADDRESS_H
