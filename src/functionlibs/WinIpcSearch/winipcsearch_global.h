#ifndef WINIPCSEARCH_GLOBAL_H
#define WINIPCSEARCH_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QVariantMap>
typedef int (__cdecl *EventCallBack)(QString,QVariantMap,void *);
typedef struct __tagEventCB{
	EventCallBack evCBName;
	void*         pUser;
}EventCBInfo, *lpEventCBInfo;

#endif // WINIPCSEARCH_GLOBAL_H
