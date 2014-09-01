#pragma once
#include <QString>
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
	bool updateRecordDatabase(int nId);
	bool updateSearchDatabase(int nId);
	bool createSearchDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,uint &uiItemId);
	bool createRecordDatabaseItem(int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,uint &uiItemId);
};

