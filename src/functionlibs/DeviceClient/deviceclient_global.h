#ifndef DEVICECLIENT_GLOBAL_H
#define DEVICECLIENT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QVariantMap>
#include <QThread>



#pragma pack()
typedef int (__cdecl *DeviceClientEventCB)(QString name,QVariantMap info,void *pUser);

typedef struct _tagDeviceClientInfoItem{
	DeviceClientEventCB proc;
	void *puser;
}DeviceClientInfoItem;



typedef struct _tagRecordVedioStream{
	uint uiLength;
	char cFrameType;
	char cChannel;
	uint uiWidth;
	uint uiHeight;
	uint uiFrameRate;
	quint64 ui64TSP;
	uint uiGenTime;
	QByteArray sData;
}RecordVedioStream;

typedef struct _tagRecordAudioStream{
	uint uiLength;
	char cFrameType;
	char cChannel;
	uint uiAudioSampleRate;
	char cAudioFormat[8];
	uint uiAudioDataWidth;
	quint64 ui64TSP;
	uint uiGenTime;
	QByteArray sData;
}RecordAudioStream;

typedef struct _tagRecordStreamFrame{
	uint uiLength;
	char cFrameType;
	char cChannel;
	union{
		uint uiAudioSampleRate;
		uint uiWidth;
	};
	union{
		char cAudioFormat[8];
		uint uiHeight;
	};
	union{
		uint uiAudioDataWidth;
		uint uiFrameRate;
	};
	quint64 ui64TSP;
	uint uiGenTime;
	char* pData;
}RecordStreamFrame;

#endif // DEVICECLIENT_GLOBAL_H
