#pragma once
#include "IDisksSetting.h"
#include <QStringList>
#include <QMutex>
#include <QtSql>
#include <QList>
typedef struct _tagPeriod{
	uint start;
	uint end;
}Period;

class StorageMgr
{
	IDisksSetting* m_pDisksSetting;

public:
	StorageMgr(void);
	~StorageMgr(void);
	//disks setting
	int getFilePackageSize();
	QString getUseDisks();
	bool getLoopRecording();
	int getFreeSizeForDisk();
	bool freeDisk();
	int getInsertId();
	int getBlockPosition();

	QString getFileSavePath(QString devname,int nChannelNum,int winId, int type, QTime &start);
	bool GetDiskFreeSpace(char* lpDirectoryName, quint64* lpFreeBytesAvailableToCaller, quint64* lpTotalNumberOfBytes, quint64* lpTotalNumberOfFreeBytes);

	//database operate
	bool createTable(QString sPath);
	bool updateRecord(QString sEnd, int size);
	bool deleteRecord(QString sFilePath);
	bool addSearchRecord(int wndId, int type, QString sDate, QString sStart, QString sEnd);
	bool updateSearchRecord(QString sEnd);
	bool deleteSearchRecord();
	/*QString getNewestRecord(QString devname, int chl);*/
private:
	QStringList findEarlestRecord(QString dbPath, QDate &earlestDate, QMap<int, QString> &maxEndTimeMap);
	void deleteRecord(QString dbPath, QString date, QMap<int, QString> &maxEndTimeMap);
	bool createSearchRecordTable();
	void deductPeriod(QString dbpath, QMap<int, QString> &maxEndTimeMap, QString date);
	void deductPeriod(int wndId, QString date, QString newEnd);
private:
	QString getUsableDisk();
	bool deleteOldDir(const QStringList& dirlist);
	QStringList deleteFile(const QStringList& fileList);
	QDate minDate(QList<QDate> dateList);
	static QMutex m_sLock;

	typedef struct _tagRecInfo{
		QString dbPath;
		QStringList fileLsit;
		QMap<int, QString> maxEndTimeMap;
	}RecInfo;

	int m_insertId;
	QString m_curDisk;
	//QString m_connectId;
	/*QString m_connectSearchId;*/
	/*QSqlDatabase *m_db;*/
	/*QSqlDatabase *m_dbSearch;*/
	static QMutex m_dblock;
	static QList<int > m_insertIdList;
	/*static QMutex m_schRecLock;*/
	int m_searchRecordId;
	int m_nPosition;
};

