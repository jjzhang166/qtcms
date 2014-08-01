#ifndef BUBBLEPROTOCOLEX_GLOBAL_H
#define BUBBLEPROTOCOLEX_GLOBAL_H

#include <QtCore/qglobal.h>

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
#pragma pack()
#endif // BUBBLEPROTOCOLEX_GLOBAL_H
