#ifndef ONVIFPROTOCOL_GLOBAL_H
#define ONVIFPROTOCOL_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QVariantMap>
#pragma  pack(1)
typedef int (__cdecl *EventHook)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagOnvifProInfo{
	EventHook proc;
	void *pUser;
}tagOnvifProInfo;
typedef struct __tagDeviceInfo{
	QString sIpAddr;
	QVariantMap vPorts;
	QString sEseeId;
	QString sUsername;
	QString sPassword;
}DeviceInfo;

typedef enum __tagConnectStatus{
	CONNECT_STATUS_DISCONNECTED,
	CONNECT_STATUS_CONNECTTING,
	CONNECT_STATUS_CONNECTED,
	CONNECT_STATUS_DISCONNECTING
}ConnectStatus;
typedef enum __tagStreamType{
	MAIN_STREAM,
	SUB_STREAM,
	OHTER_STREAM
}StreamType;
typedef enum __tagFrameType{
	TYPE_AUDIO,
	TYPE_VEDIO
}FrameType;
typedef enum __tagBubblePtzAction{
	Ptz_Up,
	Ptz_Down,
	Ptz_Left,
	Ptz_Right,
	Ptz_IrisOpen,
	Ptz_IrisClose,
	Ptz_FocusFar,
	Ptz_FocusNear,
	Ptz_ZoomIn,
	Ptz_ZoomOut,
	Ptz_Auto,
	Ptz_Stop
}tagBubblePtzAction;
#pragma pack()
#endif // ONVIFPROTOCOL_GLOBAL_H
