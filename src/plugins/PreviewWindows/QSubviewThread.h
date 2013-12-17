#pragma once
#include <QThread>
#include <guid.h>
#include "PreviewWindowsGlobalSetting.h"
#include <IDeviceClient.h>
class QSubviewThread:public QThread
{
	Q_OBJECT

public:
	QSubviewThread(void);
	~QSubviewThread(void);

	int SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname,const QString & sVendor);

	int SetDeviceClient(IDeviceClient *m_IDeviceClient);

public:
	DevCliSetInfo m_DevCliSetInfo;

public slots:
	void OpenCameraInWnd();
	void CloseAll();

private:
	IDeviceClient *m_IDeviceClient;
};

