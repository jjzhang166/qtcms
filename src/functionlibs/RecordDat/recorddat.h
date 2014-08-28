#ifndef RECORDDAT_H
#define RECORDDAT_H

#include "recorddat_global.h"
#include <IEventRegister.h>
#include <QMutex>
#include <QMultiMap>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QTime>
#include <IRecordDat.h>
#include <ISetRecordTime.h>
#include "BufferQueue.h"
#include "recordDatCore.h"
#include "recorddat_global.h"
#include <QList>

typedef int (__cdecl *recordDatEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagRecordDatProcInfo{
	recordDatEventCb proc;
	void *pUser;
}tagRecordDatProcInfo;
typedef struct __tagRecordDatTimeInfo{
	int nEnable;
	int nWeekDay;
	QTime tStartTime;
	QTime tEndTime;
}tagRecordDatTimeInfo;
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

	virtual bool init(int nWid);//init()和deinit()必须是在同一个线程中调用，不支持跨线程
	virtual bool deinit();
	virtual int inputFrame(QVariantMap &tFrameInfo);
	virtual bool manualRecordStart();
	virtual bool manualRecordStop();
	virtual bool motionRecordStart(int nTime);//单位为秒
	virtual int getRecordStatus();
	virtual bool updateRecordSchedule(int nChannelId );
private:
	void eventProcCall(QString sEvent,QVariantMap tInfo);
	bool checkMotionRecordSchedule();
	bool setRecordType(int nType,bool bFlags);//4:移动录像，2：手动录像，1：定时录像
private slots:
	void slCheckTimeRecord();
	void slCheckMotionRecord();
private:
	QStringList m_tEventList;
	QMutex m_csRef;
	int m_nRef;
	QMultiMap <QString,tagRecordDatProcInfo> m_tEventMap;
	int m_nStatus;
	int m_nWnd;
	volatile bool m_bInit;
	QMutex m_tFuncLock;
	QMutex m_tSetRecordTypeLock;
	QMutex m_tRecordDatTimeListLock;
	volatile int m_nMotionTime;
	QTimer m_tTimeRecordTimer;
	QTimer m_tMotionRecordTimer;
	QList<tagRecordDatTimeInfo> m_tRecordDatTimeList;//现阶段 定时录像与移动侦测录像共用一个录像表格
	BufferQueue m_tBufferQueue;
};

#endif // RECORDDAT_H
