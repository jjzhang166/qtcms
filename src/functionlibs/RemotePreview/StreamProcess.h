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
	bool getSupportState();
	int getStreamListInfo(QList<QList<Stream>> &lstStreamList);
	~StreamProcess(void);
public slots:
    void stopStream();

private slots:
	void receiveStream();
	void showError(QAbstractSocket::SocketError sockerror);
	void stateChanged (QAbstractSocket::SocketState socketState);
    void conToHost(QString , quint16 );
	void socketWrites(QByteArray block);
private:
	bool m_bIsHead;
	QHostAddress m_hostAddress;
	int m_nPort;
	QTcpSocket *m_tcpSocket;
	QByteArray m_buffer;
	int m_nTotalBytes;
	int m_nRemainBytes;
	QStringList m_eventList;
	QMultiMap<QString, ProcInfoItem> m_eventMap;
	int m_nVerifyResult;
	bool m_bIsSupportBubble;
	QList<QList<Stream>> m_lstStreamList;

private:
	void eventProcCall(QString sEvent,QVariantMap param);
	void analyzeStream();
	void analyzeBubbleInfo();
	QString checkXML(QString source);

};

