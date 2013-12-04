#ifndef DEVICECLIENT_GLOBAL_H
#define DEVICECLIENT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QVariantMap>

#pragma pack()
typedef int (__cdecl *DeviceClientEventCB)(QString name,QVariantMap info,void *pUser);

typedef struct _tagDeviceClientInfoItem{
	DeviceClientEventCB proc;
	void *puser;
}DeviceClientInfoItem;

#endif // DEVICECLIENT_GLOBAL_H
