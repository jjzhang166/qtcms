#include "OperationDatabase.h"


OperationDatabase::OperationDatabase(void)
{
}


OperationDatabase::~OperationDatabase(void)
{
}

QString OperationDatabase::getLatestItem( QString sDisk )
{
	return "";
}

QString OperationDatabase::createLatestItem( QString sDisk )
{
	return "";
}

bool OperationDatabase::setFileIsLock( QString sFilePath,bool bFlags )
{
	return false;
}

void OperationDatabase::clearInfoInDatabase( QString sFilePath )
{

}

bool OperationDatabase::updateRecordDatabase( int nId,QVariantMap tInfo )
{
	return false;
}

bool OperationDatabase::updateSearchDatabase( int nId ,QVariantMap tInfo)
{
	return false;
}

bool OperationDatabase::createSearchDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType ,uint &uiItemId)
{
	return false;
}

bool OperationDatabase::createRecordDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName ,uint &uiItemId)
{
	return false;
}
