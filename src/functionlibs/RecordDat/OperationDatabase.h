#pragma once
#include <QString>
#include <QVariantMap>
class OperationDatabase
{
public:
	OperationDatabase(void);
	~OperationDatabase(void);
public:
	QString getLatestItem(QString sDisk);
	QString createLatestItem(QString sDisk);
	bool setFileIsLock(QString sFilePath,bool bFlags);
	void clearInfoInDatabase(QString sFilePath);
	bool updateRecordDatabase(int nId,QVariantMap tInfo);//uiEndTime,uiType
	bool updateSearchDatabase(int nId,QVariantMap tInfo);//uiEndTime,uiType
	bool createSearchDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,uint &uiItemId);
	bool createRecordDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,uint &uiItemId);
};

