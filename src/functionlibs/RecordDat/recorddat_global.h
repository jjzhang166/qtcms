#ifndef RECORDDAT_GLOBAL_H
#define RECORDDAT_GLOBAL_H

#include <QtCore/qglobal.h>

#define MANUALRECORD 2
#define MOTIONRECORD 4
#define TIMERECORD 1
#define  WNDMAXSIZE 64
#define  IFRAME 0x01
#define  PFRAME 0x02
#define  AFRMAE 0x00
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
typedef struct __tagVideoConfigFrame{
	unsigned int uiWidth;
	unsigned int uiHeight;
	unsigned char ucVideoDec[4];
	unsigned char ucReversed[4];
}tagVideoConfigFrame;
typedef struct __tagAudioConfigFrame{
	unsigned int uiSamplebit;
	unsigned int uiSamplerate;
	unsigned int uiChannels;
	unsigned char ucAudioDec[4];
}tagAudioConfigFrame;
typedef struct __tagIFrameIndex{
	unsigned int uiFirstIFrame[64];
}tagIFrameIndex;
typedef struct __tagPerFrameIndex{
	unsigned int uiPreFrame;
	unsigned int uiPreIFrame;
	unsigned int uiNextFrame;
	unsigned int uiNextIFrame;
}tagPerFrameIndex;
#endif // RECORDDAT_GLOBAL_H
