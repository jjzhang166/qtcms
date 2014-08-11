#pragma once
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QtSql>
#include <IDisksSetting.h>
#include <QMap>
#include <QList>
typedef struct __tagMgrRecInfo{
	QString sDbPath;
	QStringList tFileList;
	QMap<int ,QString> tMaxEndTimeMap;
}tagMgrRecInfo;
typedef struct __tagStorageMgrExInfo{
	unsigned int uiRecordDataBaseId;
	unsigned int uiSearchDataBaseId;
	QString sRecordFilePath;
	QString sAllRecordDisks;
	bool bRecoverRecorder;
	int iFileMaxSize;
	int iDiskReservedSize;
}tagStorageMgrExInfo;
typedef enum __tagStorageMgrExStepCode{
	MGR_APPLYDISKSPACE,
	MGR_CREATERECORDITEM,
	MGR_CREATESEARCHITEM,
	MGR_UPDATASYSTEMDATABASE,
	MGR_DEFAULT,
	MGR_END,
}tagStorageMgrExStepCode;
class StorageMgrEx:public QThread
{
	Q_OBJECT
public:
	StorageMgrEx(void);
	~StorageMgrEx(void);
public:
	void startMgr();
	void stopMgr();
	bool applyDiskSpace();
	bool createRecordItem(unsigned int &uiItem,QString &sFilePath);
	bool createSearchItem(unsigned int &uiItem);
	tagStorageMgrExInfo getStorageMgrExInfo();
protected:
	void run();
private:
	void sleepEx(int iTime);
	bool priApplyDiskSpace();
	bool freeDisk(QString &sDisk);
	bool priCreateRecordItem();
	bool priCreateSearchItem();
	bool priUpdateSystemDataBaseData();
	QStringList findEarliestRecord(QString tDbPath,QDate &tEarlestDate,QMap<int ,QString>&tMaxEndTimeMap);
	QDate minDate(QList<QDate> tDateList);
	QStringList removeFile(QStringList tItemList);
	bool removeRecordDataBaseItem(QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo);
	bool removeSearchDataBaseItem(QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo);
private slots:
	void slCheckBlock();
private:
	bool m_bStop;
	volatile bool m_bExecuteFlag;
	QQueue<tagStorageMgrExStepCode> m_tStepCode;
	QMutex m_tFuncLock;
	QMutex m_tGetMgrInfoLock;
	int m_iSleepSwitch;
	int m_iPosition;
	volatile bool m_bIsExecute;
	tagStorageMgrExInfo m_tStorageMgrExInfo;
	QTimer m_tCheckBlockTimer;
	bool m_bIsBlock;
	IDisksSetting *m_pDisksSetting;
	QList<int> m_tCurrentUseRecordId;
};

