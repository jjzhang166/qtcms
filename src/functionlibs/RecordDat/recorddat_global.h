#ifndef RECORDDAT_GLOBAL_H
#define RECORDDAT_GLOBAL_H

#include <QtCore/qglobal.h>

#define MANUALRECORD 2
#define MOTIONRECORD 4
#define TIMERECORD 1
#define  WNDMAXSIZE 64
typedef struct __tagFrameHead{
	unsigned int uiType;
	unsigned int uiLength;
	unsigned int uiChannel;
	unsigned int uiPts;
	unsigned int uiGentime;
	unsigned int uiRecType;
	unsigned int uiExtension;
	unsigned int uiSessionId;
	char *pBuffer;
}tagFrameHead;
typedef struct __tagFileHead{
	unsigned char ucMagic[4];//JUAN
	unsigned int uiVersion;
	unsigned int uiChannels[2];
	unsigned int uiStart;
	unsigned int uiEnd;
	unsigned int uiIndex;
}tagFileHead;
#endif // RECORDDAT_GLOBAL_H
