#ifndef CHECKRECORDDATFILE_GLOBAL_H
#define CHECKRECORDDATFILE_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QList>
#define DATFILESIZE 128
#define WNDNUM 64
#define FRAMEMAXLENGTH 3
#define MANUALRECORD 2
#define MOTIONRECORD 4
#define TIMERECORD 1
typedef struct __tagIFrameIndex{
	unsigned int uiFirstIFrame[WNDNUM];//I÷°µƒ≈‰÷√÷°Œª÷√
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
typedef struct __tagRecordItemInfo{
	int nWndId;
	unsigned int uiRecordType;
	quint64 uiStartTime;
	quint64 uiEndTime;
	QString sFilePath;
}tagRecordItemInfo;

typedef struct __tagWndRecordItemInfo{
	QList<tagRecordItemInfo> tRecordItemList;
	unsigned int uiWndId;
	unsigned int uiHisRecordId;
	QString sFilePath;
	unsigned int uiFristIFrameIndex;
	unsigned int uiHistoryFrameIndex;
	unsigned int uiHistoryIFrameIndex;
	unsigned int uiNextFrameIndex;
	unsigned int uiNextIFrameIndex;
}tagWndRecordItemInfo;

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
typedef enum __tagFrameType{
	FT_Audio,//00
	FT_IFrame,//01
	FT_PFrame,//02
	FT_BFrame,
	FT_AudioConfig,
	FT_VideoConfig
}tagFrameType;
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
typedef struct __tagSearch_recordItem{
	quint64 uiStartTime;
	quint64 uiEndTime;
	unsigned int uiRecordType;
	unsigned int uiWnd;
}tagSearch_recordItem;
#endif // CHECKRECORDDATFILE_GLOBAL_H