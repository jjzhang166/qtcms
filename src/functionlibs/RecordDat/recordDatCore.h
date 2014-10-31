#pragma once
#include <QThread>
#include "BufferQueue.h"
#include <QMap>
#include <QMultiMap>
#include <QMutex>
#include <QStringList>
#include <QDebug>
#include <QTimer>
#include "recorddat_global.h"
#include "OperationDatabase.h"
#include <QFile>
#include <QByteArray>
#include <QIODevice>
#include "WriteToDisk.h"
#include <IDisksSetting.h>
#include <QEventLoop>
#include <QList>
#define OVERWRITE 0
#define  ADDWRITE 1

typedef int (__cdecl *recordDatCoreEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagRecordDatCoreProcInfo{
	recordDatCoreEventCb proc;
	void *pUser;
}tagRecordDatCoreProcInfo;
typedef struct __tagRecordDatCoreWndInfo{
	unsigned int uiHistoryRecordType;
	unsigned int uiCurrentRecordType;
	unsigned int uiPreFrame;
	unsigned int uiPreIFrame;
	unsigned int uiChannelInDatabaseId;
}tagRecordDatCoreWndInfo;
typedef struct __tagRecordDatCoreFileInfo{
	QString sFilePath;
	QMap<int ,tagRecordDatCoreWndInfo> tWndInfo;
	QMap<int ,int> tFristIFrameIndex;
	QMap<int ,int> tHistoryFrameIndex;
	QMap<int ,int >tHistoryIFrameIndex;
}tagRecordDatCoreFileInfo;
typedef struct __tagRecordDatDatabaseInfo{
	QList<int > tRemoveChannel;
	QMap<int ,int> tChannelInRecordDatabaseId;
	QMap<int ,int> tChannelInSearchDatabaseId;
}tagRecordDatDatabaseInfo;
typedef struct __tagRecordDatabaseMaxID{
	quint64 uiMaxRecordId;
	quint64 uiMaxSearchId;
}tagRecordDatabaseMaxID;
typedef enum __tagRecordDatStepCode{
	recordDat_init,//各项参数初始化
	recordDat_filePath,//查找写文件的路径
	recordDat_initMemory,//初始化内存块
	recordDat_writeMemory,//数据帧写到内存块里，数据库条目更新
	recordDat_writeDisk,//内存块写到磁盘
	recordDat_default,//检测是否需要写到磁盘，检测是否有数据帧到达
	recordDat_error,//出错
	recordDat_reset,//出错后重启
	recordDat_end//结束
}tagRecordDatStepCode;
typedef enum __tagRecordDatWriteToBufferStepCode{
	WriteToBuffer_Init,
	WriteToBuffer_00,//historyType==currentType==0,无任何操作
	WriteToBuffer_01,//historyType==0,currentType!=0,开始录像，需要等待I帧
	WriteToBuffer_10,//historyType!=0,currentType==0,停止录像
	WriteToBuffer_011,//historyType==currentType!=0,类型没有转变，接着录像
	WriteToBuffer_111,//historyType!=currentType!=0,类型转换，接着录像
	WriteToBuffer_Write,//写一帧数据
	WriteToBuffer_end,
}tagRecordDatWriteToBufferStepCode;
typedef enum __tagRecordDatReset{
	recordDatReset_fileError,//文件错误
	recordDatReset_outOfDisk,//磁盘空间不足
	recordDatReset_searchDatabase,//搜索数据库操作失败
	recordDatReset_recordDatabase,//录像数据库操作失败
	recordDatReset_writeToDisk,//写磁盘失败
}tagRecordDatReset;
typedef enum __tagRecordDatToDiskType{
	recordDatToDiskType_null,//不操作
	recordDatToDiskType_outOfTime,//定时写硬盘
	recordDatToDiskType_bufferFull,//缓存满写硬盘
}tagRecordDatToDiskType;

typedef enum __tagRecordDatTurnType{
	recordDatTurnType_noRecord,//historyType==currentType==0,无任何操作
	recordDatTurnType_beginRecord,//historyType==0,currentType!=0,开始录像，需要等待I帧
	recordDatTurnType_stopRecord,//historyType!=0,currentType==0,停止录像
	recordDatTurnType_noTurn,//historyType==currentType!=0,类型没有转变，接着录像
	recordDatTurnType_turnType//historyType!=currentType!=0,类型转换，接着录像
}tagRecordDatTurnType;
class recordDatCore:public QThread
{
	Q_OBJECT
public:
	recordDatCore(void);
	~recordDatCore(void);
public:
	bool startRecord();
	bool setBufferQueue(int nWnd,BufferQueue &tBufferQueue);
	bool removeBufferQueue(int nWnd);
	bool setRecordType(int nWnd,int nType,bool bFlags);
	void registerEvent(QString sEventName,int(__cdecl *proc)(QString,QVariantMap,void*),void *pUser);
	void reloadSystemDatabase();
protected:
	void run();
private slots:
	void slcheckBlock();
	void slsetWriteDiskFlag();
private:
	void eventCallBack(QString sEventName,QVariantMap tInfo);
	int obtainFilePath(QString &sWriteFilePath);//0:覆盖写；1：续写文件；2：没有文件可写
	//QString getUsableDisk(QString &sDiskLisk);//返回值：有剩余空间可用的盘符；传进参数：录像盘符列表
	//QString getLatestItem(QString sDisk);//参数格式：D：
	/*bool checkFileIsFull(QString sFilePath);*/
	/*bool createNewFile(QString sFilePath);*/
	bool getIsRecover();
	void sleepEx(quint64 uiTime);
	bool updateSearchDatabase();
	bool updateRecordDatabase(int nUpdateType);
	bool createSearchDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId);
	bool createRecordDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId);
	bool writeTodisk();
	void setChannelNumInFileHead();
	void setFileStartTime(quint64 &uiStartTime,quint64 &uiEndTime);
	int writeToBuffer(int nChannel,QString sFilePath);//返回值（按位）：00：buffer未满&&没写入buffer；01：buffer未满&&写入buffer；10：buffer已满&&未写入buffer；11：buffer已满&&写入buffer
private:
	QMap<int ,BufferQueue *> m_tBufferQueueMap;
	tagRecordDatCoreFileInfo m_tFileInfo;
	QMutex m_tBufferQueueMapLock;
	volatile bool m_bStop;
	QStringList m_tEventNameList;
	QMultiMap<QString ,tagRecordDatCoreProcInfo> m_tEventMap;
	int m_nPosition;
	QTimer m_tCheckIsBlockTimer;
	QTimer m_tWriteDiskTimer;
	bool m_bIsBlock;
	OperationDatabase m_tOperationDatabase;
	QByteArray m_tFileHead;
	char *m_pDataBuffer1;
	char *m_pDataBuffer2;
	char *m_pDataBuffer;
	tagRecordDatReset m_tResetType;
	tagRecordDatToDiskType m_tToDiskType;
	int m_nSleepSwitch;
	int m_nWriteMemoryChannel;
	bool m_bWriteDiskTimeFlags;
	int m_nWriteDiskTimeCount;
	tagRecordDatDatabaseInfo m_tDatabaseInfo;
	WriteToDisk m_tWriteToDisk;
	volatile bool m_bReloadSystemDatabase;
	QMultiMap<QString,tagRecordDatabaseMaxID> m_tRecordDatabaseMaxId;
	QList<quint64> m_tInsertSearchDatabaseTime;
	QMap<quint64,tagRecordItem> m_tRecordItem;
};

