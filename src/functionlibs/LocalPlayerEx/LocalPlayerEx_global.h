#ifndef LOCALPLAYEREX_GLOBAL_H
#define LOCALPLAYEREX_GLOBAL_H

#include <QtCore/qglobal.h>

#define  NO_WINDOW_ID -1
#define  MAX_BUFF_NUM 4
#define  MAX_PLAY_THREAD 4
#define  BUFFER_SIZE 120*1024*1024
#define  WNDMAX_SIZE 64
#define  MAX_FRAME_NUM 100
#define  MIN_FRAME_NUM 20
#define  MAX_SECONDS 24*60*60

#define  A_FRMAE 0x00
#define  I_FRAME 0x01
#define  P_FRAME 0x02

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

typedef struct __tagFrameHead{
	unsigned int uiType;
	unsigned int uiLength;
	unsigned int uiChannel;//¥∞ø⁄∫≈
	unsigned int uiPts;
	unsigned int uiGentime;
	unsigned int uiRecType;
	unsigned int uiExtension;
	unsigned int uiSessionId;
	char *pBuffer;
}tagFrameHead;

typedef struct __tagFrameData{
	__tagFrameData(){}
	__tagFrameData(tagFrameHead *pframeHead)
	{
		uiType = pframeHead->uiType;
		uiLength = pframeHead->uiLength;
		uiChannel = pframeHead->uiChannel;
		uiPts = pframeHead->uiPts;
		uiGentime = pframeHead->uiGentime;
		uiRecType = pframeHead->uiRecType;
		uiExtension = pframeHead->uiExtension;
		uiSessionId = pframeHead->uiSessionId;
		pBuffer = new char[pframeHead->uiLength];
		memcpy(pBuffer, &(pframeHead->pBuffer), pframeHead->uiLength);
	}
	unsigned int uiType;
	unsigned int uiLength;
	unsigned int uiChannel;//¥∞ø⁄∫≈
	unsigned int uiPts;
	unsigned int uiGentime;
	unsigned int uiRecType;
	unsigned int uiExtension;
	unsigned int uiSessionId;
	union{
		tagVideoConfigFrame VideoConfig;
		tagAudioConfigFrame AudioConfig;
	};
	char *pBuffer;
}FrameData;


typedef struct __tagIFrameIndex{
	unsigned int uiFirstIFrame[64];//I÷°µƒ≈‰÷√÷°Œª÷√
}tagIFrameIndex;

typedef struct __tagFileHead{
	unsigned char ucMagic[4];//JUAN
	unsigned int uiVersion;
	unsigned int uiChannels[2];
	unsigned int uiStart;
	unsigned int uiEnd;
	unsigned int uiIndex;
	tagIFrameIndex tIFrameIndex;
}tagFileHead;

typedef struct __tagPerFrameIndex{
	unsigned int uiPreFrame;
	unsigned int uiPreIFrame;//I÷°µƒ≈‰÷√÷°Œª÷√
	unsigned int uiNextFrame;
	unsigned int uiNextIFrame;//I÷°µƒ≈‰÷√÷°Œª÷√
}tagPerFrameIndex;

typedef struct __tagFileFrameHead{
	tagPerFrameIndex tPerFrameIndex;
	tagFrameHead tFrameHead;
}tagFileFrameHead;

typedef enum __tagFrameType{
	FT_Audio,//00
	FT_IFrame,//01
	FT_PFrame,//02
	FT_BFrame,
	FT_AudioConfig,
	FT_VideoConfig
}tagFrameType;

typedef void (*pcbTimeChange)(QString evName, uint playTime, void* pUser);


#endif // LOCALPLAYEREX_GLOBAL_H
