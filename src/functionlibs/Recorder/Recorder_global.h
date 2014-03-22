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
#endif // RECORDER_GLOBAL_H
