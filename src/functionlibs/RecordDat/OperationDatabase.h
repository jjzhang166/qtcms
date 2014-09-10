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
//使用安全条件
//1.单盘大小不能超过：256*256*256*128M
//2.
#define MAXFILENUM 256
typedef struct __tagSystemDatabaseInfo{
	QString sAllRecordDisk;//所有的录像盘符
	volatile bool bIsRecover;//是否覆盖录像
	volatile quint64 uiDiskReservedSize;//磁盘保留空间
}tagSystemDatabaseInfo;
class OperationDatabase
{
public:
	OperationDatabase(void);
	~OperationDatabase(void);
public:
	void reloadSystemDatabase();
	QString getUsableDisk(QString &sDiskLisk);//返回值：有剩余空间可用的盘符；传进参数：录像盘符列表
	QString getLatestItem(QString sDisk);//d:
	QString getOldestItem(QString sDisk);//d:
	QString createLatestItem(QString sDisk);//用于磁盘还有空间，递增模式
	void clearInfoInDatabase(QString sFilePath);
	bool updateRecordDatabase(int nId,QVariantMap tInfo,QString sFilePath);//uiEndTime,uiType
	bool updateSearchDatabase(int nId,QVariantMap tInfo,QString sFilePath);//uiEndTime,uiType
	bool createSearchDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,uint &uiItemId);
	bool createRecordDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,uint &uiItemId);
	void setRecordFileStatus(QString sFilePath,QVariantMap tInfo);
	bool getIsRecover();
	tagSystemDatabaseInfo getSystemDatabaseInfo();
	bool isDiskSpaceOverReservedSize();
	bool isRecordDataExistItem();
private:
	bool createRecordDatabase(QString sDatabasePath);
	void priSetRecordFileStatus(QString sFilePath,QVariantMap tInfo);
	quint64 countFileNum(QString sFilePath);
private:
	IDisksSetting *m_pDisksSetting;
	tagSystemDatabaseInfo m_tSystemDatabaseInfo;
	QMap<QString,QMap<int ,QString>> m_tDeleteFileList;
	freeDisk m_tFreeDisk;
};

