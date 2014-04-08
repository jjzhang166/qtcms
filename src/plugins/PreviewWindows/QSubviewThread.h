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

	//int SetDeviceClient(IDeviceClient *m_IDeviceClient);
	IDeviceClient* GetDeviceClient();
	bool GetCreateDeviceFlags();
	void SetCreateDeviceFlags(bool flags);
public:
	DevCliSetInfo m_DevCliSetInfo;
	bool bCreateDeviceFlags;

public slots:
	void OpenCameraInWnd();
	void CloseAll();
	int SetDeviceByVendor(QString sVendor, QWidget *pWnd);
private:
	IDeviceClient *m_IDeviceClient;

};

