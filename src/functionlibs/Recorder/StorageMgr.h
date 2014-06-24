#pragma once
#include "IDisksSetting.h"
#include <QStringList>
#include <QMutex>

class StorageMgr
{
	IDisksSetting* m_pDisksSetting;
	char m_currdisk;
	char m_usdisks[16];
public:
	StorageMgr(void);
	~StorageMgr(void);
	//disks setting
	int getFilePackageSize();
	QString getUseDisks();
	bool getLoopRecording();
	int getFreeSizeForDisk();
	bool freeDisk();
	//

	QString getFileSavePath(QString devname,int nChannelNum);

	bool GetDiskFreeSpaceEx(char* lpDirectoryName, quint64* lpFreeBytesAvailableToCaller, quint64* lpTotalNumberOfBytes, quint64* lpTotalNumberOfFreeBytes);
private:
	QString getUsableDisk();
	void deleteOldDir(const QStringList& dirlist);
	bool deleteDir(const QString& diskslist);
	static QMutex m_sLock;
};

