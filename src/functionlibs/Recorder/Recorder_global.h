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
	volatile unsigned int uiRecordDataBaseId;
	volatile unsigned int uiSearchDataBaseId;
	QString sRecordFilePath;
	QString sAllRecordDisks;//e:
	QString sCreateRecordItemDisk;
	volatile int iCreateRecordItemWindId;
	volatile int iCreateRecordItemType;
	volatile int iCreateRecordItemChannelNum;
	volatile int iCreateSearchItemWindId;
	volatile int iCreateSearchItemType;
	QString sCreateSearchItemStartTime;
	QString sCreateSearchItemEndTime;
	QString sCreateRecordItemDevName;
	QString sCreateSearchItemDate;
	volatile bool bRecoverRecorder;
	volatile int iFileMaxSize;
	volatile int iDiskReservedSize;
	QString sApplyDisk;
	volatile int iUpdateRecordId;
	volatile int iUpdateSearchId;
	volatile int iUpdateFileSize;
	QString sUpdateEndTime;
	QString sUpdateDisk;
	volatile int iResetCurrentRecordId;
	volatile int iResetCurrentRecordWindId;
	volatile int sResetCurrentRecordDisk;
	volatile int iDeleteSearchDataBaseItemId;
	QString sUpdateSearchDataBaseEndTime;
	volatile int iUpdateSearchDataBaseId;
}tagStorageMgrExInfo;
#endif // RECORDER_GLOBAL_H
