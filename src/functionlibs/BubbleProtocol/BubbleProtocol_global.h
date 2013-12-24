#ifndef BUBBLEPROTOCOL_GLOBAL_H
#define BUBBLEPROTOCOL_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QList>
#include <QtCore/QStringList>
#include <QByteArray>
#include <QDateTime>
#include "GlobalSettings.h"
#include <QVariantMap>
#include <QDomDocument>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

typedef struct _tagStream{
	QString sName;
	QString sSize;
	QString sx1;
	QString sx2;
	QString sx4;
}Stream;

typedef struct _tagRecord{
    char cChannel;
    char cTypes;
    QDateTime StartTime;
    QDateTime EndTime;
    QString sFileName;
}Record;

#pragma pack(1)
typedef struct _tagBubble{
	char cHead;
	unsigned int uiLength;
	char cCmd;
	unsigned int uiTicket;
	char pLoad[1]; 
}Bubble, *lpBubble;

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

typedef struct _tagRecordRequireV2{
    int      nChannels;
    int      nTypes;
    unsigned int nStart;
    unsigned int nEnd; 
}RecordRequireV2, *lpRecordRequireV2;

typedef struct _tagRecordRequire{
    unsigned int nReqListCount;
    unsigned int nChannels;
    int          nReversed;
    char         cReqList; 
}RecordRequire, *lpRecordRequire;


typedef struct _tagRecordStream{
    uint         nLength;
    char         cType;
    char         cChannel;
    int          nMagic;
    int          nSessionRnd;
    uint         nFrameWidth;
    uint         nFrameHeight;
    uint         nFrameRate;
    uint         nAudioSampleRate;
    char         cAudioFormat[8];
    uint         nAudioDataWidth;
    uint         nFrameType;
    uint         nSessionId;
    uint         nChannel;
    uint         nRecType;
    qint64       nFrameIndex;
    uint         nSize;
    qint64       nU64TSP;
    uint         nGenTime;
    char         pReversed[1];
    char         pData[1];
}RecordStream, *lpRecordStream;

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


#endif // BUBBLEPROTOCOL_GLOBAL_H
