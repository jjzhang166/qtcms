#pragma once
#include <QString>
#include <QtSql>
#include <QStringList>
#include <QDebug>
typedef struct __tagMgrRecInfo{
	QString sDbPath;
	QStringList tFileList;
	QMap<int ,QString> tMaxEndTimeMap;
}tagMgrRecInfo;
class freeDisk
{
public:
	freeDisk(void);
	~freeDisk(void);
public:
	bool freeDiskEx(QString sAllDisk,quint64 uiDiskReservedSize);//
private:
	QStringList findEarliestRecord(QString tDbPath,QDate &tEarlestDate,QMap<int ,QString>&tMaxEndTimeMap);
	bool removeRecordDataBaseItem(QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo);
	bool removeSearchDataBaseItem(QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo,QString sDate);
	QStringList removeFile(QStringList tItemList);
	QDate minDate(QList<QDate> tDateList);
};

