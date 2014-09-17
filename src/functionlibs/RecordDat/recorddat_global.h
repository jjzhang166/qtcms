#ifndef RECORDDAT_GLOBAL_H
#define RECORDDAT_GLOBAL_H

#include <vld.h>
#include <QtCore/qglobal.h>
#define MANUALRECORD 2
#define MOTIONRECORD 4
#define TIMERECORD 1
#define  WNDMAXSIZE 64
#define  IFRAME 0x01
#define  PFRAME 0x02
#define  AFRMAE 0x00
#define  BUFFERSIZE 120//单位：M
typedef struct __tagFrameHead{
	unsigned int uiType;
	unsigned int uiLength;
	unsigned int uiChannel;//窗口号
	unsigned int uiPts;
	unsigned int uiGentime;
	unsigned int uiRecType;
	unsigned int uiExtension;
	unsigned int uiSessionId;
	char *pBuffer;
}tagFrameHead;

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
	unsigned int uiFirstIFrame[64];//I帧的配置帧位置
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
	unsigned int uiPreIFrame;//I帧的配置帧位置
	unsigned int uiNextFrame;
	unsigned int uiNextIFrame;//I帧的配置帧位置
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
typedef enum __tagObtainFilePathStepCode{
	obtainFilePath_getDrive,//获取可录像盘符
	obtainFilePath_diskUsable,//有剩余空间的可录像的盘符
	obtainFilePath_diskFull,//每个盘符都已经录满
	obtainFilePath_createFile,//如果文件不存在，则创建文件
	obtainFilePath_success,//获取录像文件路径成功
	obtainFilePath_fail,//获取录像文件路径失败
	obtainFilePath_end//结束
}tagObtainFilePathStepCode;
#endif // RECORDDAT_GLOBAL_H
