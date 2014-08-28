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
};

