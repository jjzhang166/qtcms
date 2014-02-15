#ifndef IPCDEVICECLIENT_GLOBAL_H
#define IPCDEVICECLIENT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <IDeviceConnection.h>
#include <IDeviceClient.h>

#pragma pack()

typedef int (__cdecl *IpcDeviceClientEventCB)(QString name,QVariantMap info,void *pUser);

typedef struct _tagIpcDeviceClientInfoItem{
	IpcDeviceClientEventCB proc;
	void *puser;
}IpcDeviceClientInfoItem;

typedef struct _tagIpcDeviceClientToProcInfoItem{
	IpcDeviceClientEventCB proc;
	void *puser;
	int Stream;
}IpcDeviceClientToProcInfoItem;

typedef struct _tagSingleConnect{
	IDeviceConnection *m_DeviceConnecton;
}SingleConnect;
typedef struct _tagDeviceInfo{
	QString m_sAddr;
	QVariantMap m_ports;
	QString m_sEseeId;
	QString m_sUserName;
	QString m_sPassword;
	QString m_sChannelName;
}DeviceInfo;

typedef struct _tagCurStatusInfo{
	IDeviceClient::ConnectStatus m_CurStatus;
}CurStatusInfo;

#endif // IPCDEVICECLIENT_GLOBAL_H
