#ifndef REMOTEPREVIEW_GLOBAL_H
#define REMOTEPREVIEW_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QList>
#include "GlobalSettings.h"
#include <QVariantMap>


typedef struct _tagStream{
	QString sName;
	QString sSize;
	QString sx1;
	QString sx2;
	QString sx4;
}Stream;

typedef struct _tagBubbleInfo{
	QString sServer;
	QString sConnection;
	QString sVersion;
	quint32 uiChannelCount;
	QList<QList<Stream>> lstVinList;
}BubbleInfo;

#pragma pack(1)
typedef struct _tagBubble{
	char cHead;
	unsigned int uiLength;
	char cCmd;
	unsigned int uiTicket;
	char pLoad[1]; 
}Bubble;

typedef struct _tagMessage{
	unsigned int uiLength;
	char cMessage;
	char cReverse[3];
	char pParameters[1]; 
}Message;

typedef struct _tagLiveStream{
	unsigned int uiLength;
	char cType;
	char cChannel;
	char pData[1]; 
}LiveStream;

typedef struct _tagLiveStreamRequire{
	char cChannel;
	char cOperation; 
}LiveStreamRequire;

typedef struct _tagLiveStreamRequireEx{
	unsigned int uiChannel;
	unsigned int uiStream;
	unsigned int uiOperation;
	unsigned int uiReversed; 
}LiveStreamRequireEx;

typedef struct _tagLiveStreamAudio{
	unsigned int uiEntries;
	unsigned int uiPacksize;
	unsigned long long ui64Pts;
	unsigned int uiGtime;
	char sEncode[8];
	unsigned int uiSampleRate;
	unsigned int uiSampleWidth; 
}LiveStreamAudio;

typedef struct _tagAuthority{
	char sUserName[20];
	char sPassWord[20];
}Authority;

typedef struct _tagAuthorityBack{
	char cVerified;
	char sReversed[3];
	char sAuth[13];
}AuthorityBack;
#pragma pack()
typedef int (__cdecl *PreviewEventCB)(QString name, QVariantMap info, void* pUser);
typedef struct _tagProcInfoItem
{
	PreviewEventCB proc;
	void		*puser;
}ProcInfoItem;


#endif // REMOTEPREVIEW_GLOBAL_H
