#ifndef BUBBLEPROTOCOLEX_GLOBAL_H
#define BUBBLEPROTOCOLEX_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#pragma  pack(1)
typedef struct __tagBubbleInfo{
	char cHead;
	unsigned int uiLength;
	char cCmd;
	unsigned int uiTicket;
	char pLoad[1];
}tagBubbleInfo;
typedef struct __tagBubbleMessageInfo{
	unsigned int uiLength;
	char cMessage;
	char cReverse[3];
	char cParameters[1];
}tagBubbleMessageInfo;
typedef struct __tagBubbleAuthoritySend{
	char cUserName[20];
	char cPassWord[20];
}tagBubbleAuthoritySend;
typedef struct __tagBubbleAuthorityReceive{
	char cVerified;
	char sReversed[3];
	char sAuth[13];
}tagBubbleAuthorityReceive;
typedef struct __tagBubbleLiveStreamRequireEx{
	unsigned int uiChannel;
	unsigned int uiStream;
	unsigned int uiOperation;
	unsigned int uiReversed;
}tagBubbleLiveStreamRequireEx;
typedef struct __tagBubbleLiveStreamRequire{
	char cChannel;
	char cOperation;
}tagBubbleLiveStreamRequire;
typedef struct __tagBubbleRemotePlayRecordRequireV2{
	int nChannels;
	int nTypes;
	unsigned int uiStart;
	unsigned int uiEnd;
}tagBubbleRemotePlayRecordRequireV2;
typedef struct __tagBubbleHttpStreamInfo{
	QString sName;
	QString sSize;
	QString sX1;
	QString sX2;
	QString sX4;
}tagBubbleHttpStreamInfo;
typedef struct __tagBubbleRecordStream{
	uint uiLength;
	char cType;
	char cChannel;
	int nMagic;
	int nSessionRnd;
	uint uiFrameWidth;
	uint uiFrameHeight;
	uint uiFrameRate;
	uint uiAudioSampleRate;
	char cAudioFormat[8];
	uint uiAudioDataWidth;
	uint uiFrameType;
	uint uiSessionId;
	uint uiChannel;
	uint uiRecType;
	qint64 nFrameIndex;
	uint uiSize;
	qint64 nU64TSP;
	uint uiGenTime;
	char cReversed[1];
	char cData[1];
}tagBubbleRecordStream;
typedef struct __tagBubbleLiveStream{
	unsigned int uiLength;
	char cType;
	char cChannel;
	char pData[1];
}tagBubbleLiveStream;
typedef struct __tagBubbleLiveStreamAudio{
	unsigned int uiEntries;
	unsigned int uiPackSize;
	unsigned long long ui64Pts;
	unsigned int uiGtime;
	char cEnCode[8];
	unsigned int uiSampleRate;
	unsigned int uiSampleWidth;
}tagBubbleLiveStreamAudio;
typedef struct __tagBubbleReceiveMessage{
	unsigned int uiLength;
	char cMessage;
	char cReverse[3];
	char	cParameters;
}tagBubbleReceiveMessage;
#pragma pack()
#endif // BUBBLEPROTOCOLEX_GLOBAL_H
