#pragma once
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <IDisksSetting.h>
#include <QDebug>
#include <QtSql>
#include <QMap>
#include "recorddat_global.h"
#include "freeDisk.h"
#include <QThread>
#include <QQueue>
#include <QMutex>
#include "sqlite3.h"
//使用安全条件
//1.单盘大小不能超过：256*256*256*128M
//2.
#define MAXFILENUM 256
#define FILLNEWFILESIZE 1048576
typedef struct __tagSystemDatabaseInfo{
	QString sAllRecordDisk;//所有的录像盘符
	volatile bool bIsRecover;//是否覆盖录像
	volatile quint64 uiDiskReservedSize;//磁盘保留空间
}tagSystemDatabaseInfo;
typedef enum __tagOperationDatabaseCode{
	OperationDatabase_init,
	OperationDatabase_obtainFilePath,//获取录像文件路径
	OperationDatabase_updateRecordDatabase,//更新录像数据库
	OperationDatabase_updateSearchDatabase,//更新搜索数据库
	OperationDatabase_createSearchDatabaseItem,//创建搜索数据库item
	OperationDatabase_createRecordDatabaseItem,//创建录像数据库item
	OperationDatabase_setRecordFileStatus,//设置文件数据库状态
	OperationDatabase_reloadSystemDatabase,//更新系统数据库
	OperationDatabase_clearInfoInDatabase,//清空数据库内容
	OperationDatabase_isRecordDataExistItem,
	OperationDatabase_getMaxDatabaseId,
	OperationDatabase_default
}tagOperationDatabaseCode;
typedef struct __tagCodeWithParm{
	tagOperationDatabaseCode tCode;
	QVariantMap tInfo;
}tagCodeWithParm;
class OperationDatabase:public QThread
{
	Q_OBJECT
public:
	OperationDatabase(void);
	~OperationDatabase(void);
public:
	void reloadSystemDatabase();
	bool updateRecordDatabase(QList<int> tIdList,QVariantMap tInfo,QString sFilePath);//uiEndTime
	bool updateSearchDatabase(QList<int> tIdList,QVariantMap tInfo,QString sFilePath);//uiEndTime
	bool createSearchDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId);
	bool createRecordDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId);
	void setRecordFileStatus(QString sFilePath,QVariantMap tInfo);
	bool getIsRecover();
	tagSystemDatabaseInfo getSystemDatabaseInfo();
	bool isDiskSpaceOverReservedSize();
	bool isRecordDataExistItem();
	//
	int obtainFilePath(QString &sWriteFilePath);//0:覆盖写；1：续写文件；2：没有文件可写
	void clearInfoInDatabaseEx(QString sFilePath);//unuse
	bool startOperationDatabase();
	bool stopOperationDatabase();
	bool getMaxDatabaseId(quint64 &uiMaxRecordId,quint64 &uiMaxSearchId,QString sFilePath);
protected:
	void run();
private:
	bool createRecordDatabase(QString sDatabasePath);
	void priSetRecordFileStatus(QString sFilePath,QVariantMap tInfo);
	quint64 countFileNum(QString sFilePath);
	bool execCommand(QSqlQuery & tQuery,QString sCommand);
	SQLITE_API int sqlite3_exec_reTry(
		sqlite3*,                                  /* An open database */
		const char *sql,                           /* SQL to be evaluated */
		int (*callback)(void*,int,char**,char**),  /* Callback function */
		void *,                                    /* 1st argument to callback */
		char **errmsg                              /* Error msg written here */
		);
private:
	int priObtainFilePath(QString &sWriteFilePath);//0:覆盖写；1：续写文件；2：没有文件可写
	bool priUpdateRecordDatabase(QList<int> tIdList,QVariantMap tInfo,QString sFilePath);//uiEndTime
	bool priUpdateSearchDatabase(QList<int> tIdList,QVariantMap tInfo,QString sFilePath);//uiEndTime
	bool priCreateSearchDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId);
	bool priCreateRecordDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId);
	void priSetRecordFileStatusEx(QString sFilePath,QVariantMap tInfo);
	void priReloadSystemDatabase();
	void priClearInfoInDatabase(QString sFilePath);
	bool PriIsRecordDataExistItem();
	bool PriGetMaxDatabaseId(quint64 &uiMaxRecordId,quint64 &uiMaxSearchId,QString sFilePath);
	void clearInfoIndatabaseWithNativeAPIs(QString sFilePath);
	//
	bool createNewFile(QString sFilePath);
	QString createLatestItemEx(QString sDisk);//用于磁盘还有空间，递增模式
	bool checkFileIsFull(QString sFilePath);
	QString getLatestItemEx(QString sDisk);//d:
	QString getLatestItemExx(QString sDisk);//d:
	QString getOldestItemEx(QString sDisk,quint64 &uiStartTime);//d:
	//QString getOldestItemExx(QString sDisk);//d:
	QString getUsableDiskEx(QString &sDiskLisk);//返回值：有剩余空间可用的盘符；传进参数：录像盘符列表
private:
	IDisksSetting *m_pDisksSetting;
	tagSystemDatabaseInfo m_tSystemDatabaseInfo;
	QMap<QString,QMap<int ,QString>> m_tDeleteFileList;
	freeDisk m_tFreeDisk;
	QQueue<tagCodeWithParm> m_tStepCode;
	bool m_bStop;
	QByteArray m_tFileHead;
	char *m_pNewFile;
	QMutex m_tStepCodeLock;
	QMutex m_tObtainFilePathLock;
	QQueue<QVariantMap> m_tObtainFilePathResult;//sWriteFilePath nReturn
	QQueue<bool> m_tIsRecordDataExistItemResult;//nReturn
	QQueue<QVariantMap> m_tGetMaxDatabaseIdResult;//uiMaxRecordId uiMaxSearchId
	QMap<QString,sqlite3 * >m_tNativeApiList;
};

