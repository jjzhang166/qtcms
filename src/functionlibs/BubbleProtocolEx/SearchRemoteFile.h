#pragma once
#include <QDateTime>
#include <QVariantMap>
#include <QMultiMap>
#include <QStringList>
#include <QTcpSocket>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QNetworkReply>
#include <QHostAddress>
#include <QList>
#include <QDomDocument>
#include <QString>
typedef int (__cdecl *SearchRemoteFileCb)(QString eventName,QVariantMap info,void *pUser);
typedef struct __tagSearchRemoteFileProcInfo{
	SearchRemoteFileCb Proc;
	void *pUser;
}tagSearchRemoteFileProcInfo;
typedef struct __tagSearchInfo{
	int nSessionIndex;
	int nSessionCount;
	int nSessionTotal;
}tagSearchInfo;
typedef struct __tagSearchRecordInfo{
	char cChannel;
	char cTypes;
	QDateTime tStartTime;
	QDateTime tEndTime;
	QString sFileName;
}tagSearchRecordInfo;
class SearchRemoteFile
{
public:
	SearchRemoteFile(void);
	~SearchRemoteFile(void);
public:
	int startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime,QHostAddress tAddr,QVariantMap tPorts,QString sUserName,QString sPassWord);
	void registerEvent(QString eventName,int (__cdecl *proc)(QString ,QVariantMap ,void *),void *pUser);
private:
	void eventCallBack(QString sEventName,QVariantMap evMap);
	int parseSearchData();
	int extractRecordInfo(QDomDocument* pDom);
private:
	QStringList m_tEventNameList;
	QMultiMap<QString,tagSearchRemoteFileProcInfo>m_tEventMap;
	tagSearchInfo m_tSearchInfo;
	QTcpSocket m_tSearchRemoteFileTcpSocket;
	QList<tagSearchRecordInfo> m_tRecordList;
	QByteArray   m_tBlock;
};

