#ifndef _PREVIEWWINDOWSGLOBALSETTINGS_HEAD_FILE_
#define _PREVIEWWINDOWSGLOBALSETTINGS_HEAD_FILE_
#include <QMutex>
#include <QString>


extern QMutex g_PreviewWindowsMutex;

typedef struct _tagDevCliSetInfo{
	QString m_sAddress;
	unsigned int m_uiPort;
	QString m_sEseeId;
	unsigned int m_uiChannelId;
	unsigned int m_uiStreamId;
	QString m_sUsername;
	QString m_sPassword;
	QString m_sCameraname;
	QString m_sVendor;
}DevCliSetInfo;
typedef struct _tagRenderInfo{
	char* pData;
	char* pYdata;
	char* pUdata;
	char* pVdata;
	int iWidth;
	int iHeight;
	int iYStride;
	int iUVStride;
	int iLineStride;
	QString iPixeFormat;
	int iFlags;
}RenderInfo;
#endif