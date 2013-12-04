#pragma once

#include "RemotePreview_global.h"
#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QByteArray>
#include <QtCore/QStringList>
#include <QtCore/QMultiMap>

class StreamProcess : public QObject
{
	Q_OBJECT
public:
	StreamProcess(void);
	void setAddressInfo(QHostAddress hostAddress, int port);
	void setEventMap(QStringList eventList, QMultiMap<QString, ProcInfoItem> eventMap);
    int getSocketState();
	int getVerifyResult();
	~StreamProcess(void);
public slots:
    void stopStream();

private slots:
	void receiveStream();
	void showError(QAbstractSocket::SocketError sockerror);
    void conToHost(QString , quint16 );
	void socketWrites(QByteArray block);
private:
	bool m_bStop;
	bool m_bIsHead;
	QHostAddress m_hostAddress;
	int m_nPort;
	QTcpServer m_tcpServer;
	QTcpSocket *m_tcpSocket;
	QByteArray m_buffer;
	int m_nTotalBytes;
	int m_nRemainBytes;
	QStringList m_eventList;
	QMultiMap<QString, ProcInfoItem> m_eventMap;
	int m_nVerifyResult;

private:
	void eventProcCall(QString sEvent,QVariantMap param);
	void analyzeStream();

};

