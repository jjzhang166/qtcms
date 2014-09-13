#ifndef MDWORKOBJECT_H
#define MDWORKOBJECT_H

#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QMap>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QMutex>

class MDWorkObject : public QThread
{
	Q_OBJECT

public:
	MDWorkObject(QObject * parent);
	virtual ~MDWorkObject();

// struct 
public:
	typedef void (*EventProc)(QString sEvent,QVariantMap eParam,void * pUser);
	
	typedef struct _tagEventMap{
		EventProc proc;
		void * pUser;
	}EventMap;


private:
	QObject * m_workObject;

public:
	void setHostInfo(QString sAddress,unsigned int uPort);
	void setUserInfo(QString sUsername,QString sPassword);
	int startMd();
	int stopMd();

	// event
	void registerEvent(QString sEvent,EventProc proc,void * pUser);

private:
	void eventProcCall(QString sEvent,QVariantMap eParam);

	int InitSocket(QTcpSocket * s);

	virtual void run();
	int mdWorkProc(QTcpSocket * s);
private:
	QMap<QString,EventMap> m_eventMap;
	QString m_sUsername;
	QString m_sPassword;
	QString m_sAddress;
	unsigned int m_uPort;

	bool m_bQuitMd;
};

#endif // MDWORKOBJECT_H
