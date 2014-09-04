#pragma once
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QIODevice>
#include <QVariantMap>
#include <QMultiMap>
#include <QStringList>
typedef int (__cdecl *writeToDiskEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagWriteToDiskProcInfo{
	writeToDiskEventCb proc;
	void *pUser;
}tagWriteToDiskProcInfo;
class WriteToDisk:public QThread
{
	Q_OBJECT
public:
	WriteToDisk(void);
	~WriteToDisk(void);
public:
	void stopWriteToDisk();
	void startWriteToDisk(char* pBuffer,QString sFilePath,quint64 uiBufferSize);
	void registerEvent(QString sEventName,int(__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
protected:
	void run();
private slots:
	void slCheckBlock();
private:
	void sleepEx(int nTime);
	bool ensureFileExist();
	void eventCallBack(QString sEventName,QVariantMap tInfo);
private:
	volatile bool m_bStop;
	volatile bool m_bWrite;
	quint64 m_uiBufferSize;
	char* m_pBuffer;
	int m_nSleepSwitch;
	int m_nPosition;
	QTimer m_tCheckBlockTimer;
	bool m_bBlock;
	QMutex m_tBufferLock;
	QString m_sFilePath;
	QMultiMap<QString,tagWriteToDiskProcInfo> m_tEventMap;
	QStringList m_tEventList;
};

