#pragma once
#include <QObject>
#include <QThread>
#include <QDebug>
#include <IDeviceClient.h>
#include "QSubviewThread.h"
#include <qtconcurrentrun.h>
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

	void SetAutoSyncTime(bool bEnabled);
	IDeviceClient *SetDeviceByVendor(QString sVendor,QWidget *wnd);

signals:
	void OpenCameraInWndSignl();
	void m_workerThreadQuitSignal();
	void SetDeviceByVendorSignal(QString ,QWidget *);
private:
	IDeviceClient *m_IDeviceClient;
	QSubviewThread *m_QSubviewProcess;

public slots:
	void m_workerThreadQuit();
private:
	bool bIsOpen;
};

