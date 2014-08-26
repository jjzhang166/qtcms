#ifndef RECORDDAT_H
#define RECORDDAT_H

#include "recorddat_global.h"
#include <IEventRegister.h>
#include <QMutex>
#include <QMultiMap>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <IRecordDat.h>
#define MANUALRECORD 2
#define MOTIONRECORD 4
#define TIMERECORD 1
typedef int (__cdecl *recordDatEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagRecordDatProcInfo{
	recordDatEventCb proc;
	void *pUser;
}tagRecordDatProcInfo;
class  RecordDat:public QObject,
	public IRecordDat,
	public IEventRegister
{
	Q_OBJECT
public:
	RecordDat();
	~RecordDat();
public:
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	virtual bool init();//init()和deinit()必须是在同一个线程中调用，不支持跨线程
	virtual bool deinit();
	virtual int inputFrame(QVariantMap &tFrameInfo);
	virtual bool manualRecordStart();
	virtual bool manualRecordStop();
	virtual bool motionRecordStart(int nTime);//单位为秒
	virtual int getRecordStatus();
	virtual bool updateRecordSchedule(int nChannelId );
private:
	void eventProcCall(QString sEvent,QVariantMap tInfo);
	bool setRecordType(int nType,bool bFlags);//4:移动录像，2：手动录像，1：定时录像
private slots:
	void slCheckTimeRecord();
private:
	QStringList m_tEventList;
	QMutex m_csRef;
	int m_nRef;
	QMultiMap <QString,tagRecordDatProcInfo> m_tEventMap;
	int m_nStatus;
	bool m_bInit;
	QMutex m_tFuncLock;
	QMutex m_tSetRecordTypeLock;
	int m_nMotionTime;
	QTimer m_tTimeRecordTimer;
};

#endif // RECORDDAT_H
