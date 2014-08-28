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
#define OVERWRITE 0
#define  ADDWRITE 1
#define  BUFFERSIZE 120//单位：M
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
}tagRecordDatCoreFileInfo;
typedef enum __tagRecordDatStepCode{
	recordDat_init,//各项参数初始化
	recordDat_filePath,//查找写文件的路径
	recordDat_initMemory,//初始化内存块
	recordDat_writeMemory,//数据帧写到内存块里，数据库条目更新
	recordDat_writeDisk,//内存块写到磁盘
	recordDat_default,//检测是否需要写到磁盘，检测是否有数据帧到达
	recordDat_error,//出错
	recordDat_end//结束
}tagRecordDatStepCode;
typedef enum __tagObtainFilePathStepCode{
	obtainFilePath_getDrive,//获取可录像盘符
	obtainFilePath_diskUsable,//有剩余空间的可录像的盘符
	obtainFilePath_diskFull,//每个盘符都已经录满
	obtainFilePath_createFile,//如果文件不存在，则创建文件
	obtainFilePath_success,//获取录像文件路径成功
	obtainFilePath_fail,//获取录像文件路径失败
	obtainFilePath_end//结束
}tagObtainFilePathStepCode;
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
protected:
	void run();
private slots:
	void slcheckBlock();
private:
	void eventCallBack(QString sEventName,QVariantMap tInfo);
	int obtainFilePath(QString &sWriteFilePath);//0:覆盖写；1：续写文件；2：没有文件可写
	QString getUsableDisk(QString &sDiskLisk);
	QString getLatestItem(QString sDisk);
	bool checkFileIsFull(QString sFilePath);
	bool createNewFile(QString sFilePath);
private:
	QMap<int ,BufferQueue *> m_tBufferQueueMap;
	tagRecordDatCoreFileInfo m_tFileInfo;
	QMutex m_tBufferQueueMapLock;
	volatile bool m_bStop;
	QStringList m_tEventNameList;
	QMultiMap<QString ,tagRecordDatCoreProcInfo> m_tEventMap;
	int m_nPosition;
	QTimer m_tCheckIsBlockTimer;
	bool m_bIsBlock;
	OperationDatabase m_tOperationDatabase;
	QByteArray m_tFileHead;
	char *m_pDataBuffer1;
	char *m_pDataBuffer2;
	char *m_pDataBuffer;
};

