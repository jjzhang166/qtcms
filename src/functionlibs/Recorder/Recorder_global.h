#ifndef RECORDER_GLOBAL_H
#define RECORDER_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QVariantMap>

typedef int (__cdecl *PreviewEventCB)(QString name, QVariantMap info, void* pUser);
typedef struct _tagProcInfoItem
{
	PreviewEventCB proc;
	void		*puser;
}ProcInfoItem;
typedef struct __tagStorageMgrExInfo{
	unsigned int uiRecordDataBaseId;
	unsigned int uiSearchDataBaseId;
	QString sRecordFilePath;
	QString sAllRecordDisks;//e:
	QString sCreateRecordItemDisk;
	int iCreateRecordItemWindId;
	int iCreateRecordItemType;
	int iCreateRecordItemChannelNum;
	int iCreateSearchItemWindId;
	int iCreateSearchItemType;
	QString sCreateSearchItemStartTime;
	QString sCreateSearchItemEndTime;
	QString sCreateRecordItemDevName;
	QString sCreateSearchItemDate;
	bool bRecoverRecorder;
	int iFileMaxSize;
	int iDiskReservedSize;
	QString sApplyDisk;
}tagStorageMgrExInfo;
#endif // RECORDER_GLOBAL_H
