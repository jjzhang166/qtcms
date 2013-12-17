#pragma once
#include <QObject>
#include <QThread>
#include <IDeviceClient.h>
#include "QSubviewThread.h"
class QSubViewObject:public QThread
{
	Q_OBJECT
		QThread m_workerThread;
public:
	QSubViewObject(void);
	~QSubViewObject(void);

public:
	int OpenCameraInWnd();
	int CloseWndCamera();
	int SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname,const QString & sVendor);
	int SetDeviceClient(IDeviceClient *m_IDeviceClient);
signals:
	void OpenCameraInWndSignl();
	void CloseAllSignl();
private:
	IDeviceClient *m_IDeviceClient;
	QSubviewThread *m_QSubviewProcess;
};

