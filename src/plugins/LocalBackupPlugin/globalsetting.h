#include "avilib.h"
#include <QString>
#include <QMap>


#define MAX_BUFF_SIZE 64*1024*1024

typedef struct __tagVideoConfigFrame{
	unsigned int uiWidth;
	unsigned int uiHeight;
	unsigned char ucVideoDec[4];
	unsigned char ucReversed[4];
}VideoConfigFrame;

typedef struct __tagAudioConfigFrame{
	unsigned int uiSamplebit;
	unsigned int uiSamplerate;
	unsigned int uiChannels;
	unsigned char ucAudioDec[4];
}AudioConfigFrame;

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
}FrameHead;

typedef struct __tagIFrameIndex{
	unsigned int uiFirstIFrame[64];//I÷°µƒ≈‰÷√÷°Œª÷√
}IFrameIndex;

typedef struct __tagFileHead{
	unsigned char ucMagic[4];//JUAN
	unsigned int uiVersion;
	unsigned int uiChannels[2];
	unsigned int uiStart;
	unsigned int uiEnd;
	unsigned int uiIndex;
	IFrameIndex tIFrameIndex;
}FileHead;

typedef struct __tagPerFrameIndex{
	unsigned int uiPreFrame;
	unsigned int uiPreIFrame;//I÷°µƒ≈‰÷√÷°Œª÷√
	unsigned int uiNextFrame;
	unsigned int uiNextIFrame;//I÷°µƒ≈‰÷√÷°Œª÷√
}PerFrameIndex;

typedef struct __tagFileFrameHead{
	PerFrameIndex tPerFrameIndex;
	FrameHead tFrameHead;
}FileFrameHead;

typedef enum __tagFrameType{
	FT_Audio,//00
	FT_IFrame,//01
	FT_PFrame,//02
	FT_BFrame,
	FT_AudioConfig,
	FT_VideoConfig
}FrameType;

typedef enum _emStatusCode{
	EM_INIT,
	EM_READ_BUFF,
	EM_CREATE_FILE,
	EM_CHECK_SIZE,
	EM_WRITE_FRAME,
	EM_PACK,
	EM_STOP
}StatusCode;

typedef enum _emFileStatus{
	EM_NO_CREATED,
	EM_WRITABLE,
	EM_WAIT_FOR_PACK
}FileStatus;

typedef struct _tagFdStatusInfo{
	_tagFdStatusInfo(){
		fd = NULL;
		fileNum = 0;
		progress = 0;
		fileStatus = EM_NO_CREATED;
		frameCounts = 0;
		startGMT = 0;
		keyPts = 0;
		width = 0;
		height = 0;
		samplebit = 0;
		samplerate = 0;
		channel = 1;
		audioBeSet = false;
		firstFile = true;
	}
	//avi file handle
	avi_t *fd;
	//map for counting pts
	QMap<int,int> ptsCountMap;
	//part number for file
	int fileNum;
	uint startGMT;
	int progress;
	//current file status
	int fileStatus;
	uint keyPts;
	//count frames
	int frameCounts;
	//video
	int width;
	int height;
	//audio
	int samplebit;
	int samplerate;
	int channel;
	//audio set flag
	bool audioBeSet;
	//first file flag
	bool firstFile;

}FdStatusInfo;

typedef struct _tagTimePath{
	uint start;
	QString path;
}TimePath;

typedef QMap<int, FdStatusInfo>::iterator FdIterator;

