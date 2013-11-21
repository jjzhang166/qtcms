#ifndef DVRSEARCH_GLOBAL_H
#define DVRSEARCH_GLOBAL_H


#include <QtCore/qglobal.h>
#include <QVariantMap>


typedef struct __tagDvrSearchInfo{
	char sIp[32];
	char sId[32];
	unsigned short usHttp;
	unsigned short usMedia;
	unsigned int uiChannelCount;
}DvrSearchInfo,*lpDvrSearchInfo; 

#define UDP_PORT   9013
#define SENDBUFF   "SEARCHDEV"

typedef int (__cdecl *EventCallBack)(QString,QVariantMap,void *);
typedef struct __tagEventCB{
    EventCallBack evCBName;
    void*         pUser;
}EventCBInfo, *lpEventCBInfo;


#endif // DVRSEARCH_GLOBAL_H
