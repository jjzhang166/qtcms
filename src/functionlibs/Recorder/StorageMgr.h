#pragma once
#include "IDisksSetting.h"
#include <QStringList>

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
	//

	QString getFileSavePath(QString devname,int nChannelNum);

	bool GetDiskFreeSpaceEx(char* lpDirectoryName, quint64* lpFreeBytesAvailableToCaller, quint64* lpTotalNumberOfBytes, quint64* lpTotalNumberOfFreeBytes);
private:
	QString getUsableDisk();
	void deleteOldDir(const QStringList& dirlist);
	bool deleteDir(const QString& diskslist);
};

