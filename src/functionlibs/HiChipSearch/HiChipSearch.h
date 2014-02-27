#ifndef _HICHIPSEARCH_HEAD_FILE_H_
#define _HICHIPSEARCH_HEAD_FILE_H_

#include "HiChipSearch_global.h"
#include <QObject>
#include <QtNetwork/QUdpSocket>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include "IDeviceSearch.h"
#include "IDeviceNetModify.h"
#include "IEventRegister.h"

#define MCASTADDR     "239.255.255.250"
#define MCASTPORT      8002
#define BUFSIZE        1024 



class HiChipSearch : public QThread,
	public IDeviceSearch,
	public IDeviceNetModify,
	public IEventRegister
{
	Q_OBJECT
public:
	HiChipSearch();
	~HiChipSearch();

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();

	virtual int Start();
	virtual int Stop();
	virtual int Flush();
	virtual int setInterval(int nInterval);
	virtual IEventRegister * QueryEventRegister();

	virtual int SetNetworkInfo(const QString &sDeviceID,
		const QString &sAddress,
		const QString &sMask,
		const QString &sGateway,
		const QString &sMac,
		const QString &sPort,
		const QString &sUsername,
		const QString &sPassword);

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
protected:
	void run();

private:
	void Receive();
	void getItem(QByteArray, QByteArray, QString&);
	void parseSearchAck(QByteArray, QVariantMap&);
	QString GetHostAddress();
	void eventProcCall(QString sEvent,QVariantMap param);
private:
	int									m_nRef;
	QMutex								m_csRef;
	volatile bool						m_bReceiving;
	bool								m_bFlush;
	bool								m_bEnd;
	QUdpSocket							*m_Socket;
	int									m_nInterval;
	QMultiMap<QString, ProcInfoItem_t>	m_eventMap;
	QStringList							m_eventList;
	QByteArray							m_portInfo;
	QByteArray							m_netInfo;
};


#endif