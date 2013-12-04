#ifndef _REMOTEPREVIEW_HEADFILE_H_
#define _REMOTEPREVIEW_HEADFILE_H_


#include "RemotePreview_global.h"
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QNetworkReply>
#include <QThread>
#include <QDomDocument>
#include <QMutex>
#include "IRemotePreview.h"
#include "IDeviceConnection.h"
#include "IEventRegister.h"
#include "StreamProcess.h"

class RemotePreview : public QObject,
	public IEventRegister,
	public IRemotePreview,
	public IDeviceConnection
{
	Q_OBJECT
		QThread m_workerThread;
public:
	RemotePreview(void);
	~RemotePreview(void);

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//interface for device connection
	virtual int setDeviceHost(const QString & sAddr);
	virtual int setDevicePorts(const QVariantMap & ports);
	virtual int setDeviceId(const QString & sAddress);
	virtual int setDeviceAuthorityInfomation(QString username,QString password);
	virtual int connectToDevice();
	virtual int authority();
	virtual int disconnect();
	virtual int getCurrentStatus();
	virtual QString getDeviceHost();
	virtual QString getDeviceid();
	virtual QVariantMap getDevicePorts();

	//interface for remote preview
	virtual int getLiveStream(int nChannel, int nStream);
	virtual int stopStream();
	virtual int pauseStream(bool bPaused);
	virtual int getStreamCount();
	virtual int getStreamInfo(int nStreamId,QVariantMap &streamInfo);

 	//interface for event register
 	virtual QStringList eventList();
 	virtual int queryEvent(QString eventName,QStringList& eventParams);
 	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

public:
	//Custom Functions
	void sendRequire(bool bSwitch);
	void sendLiveStreamRequire(bool option);
	void sendLiveStreamRequireEx(bool option);
	QString checkXML(QString source);
	void extractStreamInfo(QDomDocument *dom);
private:
	//member variable about device connection
	QNetworkAccessManager *m_manager;
	QNetworkReply *m_reply;
	QByteArray m_block;
	QHostAddress m_hostAddress;
	QVariantMap m_ports;
	QString m_deviceId;
	QString m_deviceUsername;
	QString m_devicePassword;
	QList<QList<Stream>> m_lstStreamList;
	bool m_isSupportBubble;

	QMultiMap<QString, ProcInfoItem> m_eventMap;
	QStringList m_eventList;
	bool m_bpaused;
	int m_channelNum;
	int m_streamNum;
	int m_streanCount;

	StreamProcess *m_pStreamProcess;

	int	m_nRef;
	QMutex m_csRef;
signals:
	void QuitThread();
	void EndStream();
	void writeSocket(QByteArray block);
    void childThreadToConn(QString address, quint16 port);
private slots:
	void finishReply();
};

#endif