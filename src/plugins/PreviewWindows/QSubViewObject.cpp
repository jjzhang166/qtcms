#include "QSubViewObject.h"


QSubViewObject::QSubViewObject(void):m_IDeviceClient(NULL),
	m_QSubviewProcess(NULL)
{
	m_QSubviewProcess=new QSubviewThread;
	m_QSubviewProcess->moveToThread(&m_workerThread);
	connect(&m_workerThread, SIGNAL(finished()), m_QSubviewProcess, SLOT(deleteLater()));
	connect(this,SIGNAL(OpenCameraInWndSignl()),m_QSubviewProcess,SLOT(OpenCameraInWnd()),Qt::QueuedConnection);
	connect(this,SIGNAL(m_workerThreadQuitSignal()),this,SLOT(m_workerThreadQuit()),Qt::QueuedConnection);
	connect(this,SIGNAL(SetDeviceByVendorSignal(QString, QWidget *)),m_QSubviewProcess,SLOT(SetDeviceByVendor(QString, QWidget *)),Qt::QueuedConnection);
	/*m_QSubviewProcess->SetDeviceClient(m_IDeviceClient);*/
	m_workerThread.start();
}


QSubViewObject::~QSubViewObject(void)
{
	CloseWndCamera();
	m_workerThread.quit();
	m_workerThread.wait();
}
int QSubViewObject::OpenCameraInWnd()
{
	emit OpenCameraInWndSignl();
	return 0;
}
int QSubViewObject::CloseWndCamera()
 {
	m_QSubviewProcess->CloseAll();
	return 0;
}
int QSubViewObject::SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
	m_QSubviewProcess->SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	return 0;
}


void QSubViewObject::m_workerThreadQuit()
{
	
	m_workerThread.quit();
	m_workerThread.wait();
}

IDeviceClient* QSubViewObject::SetDeviceByVendor( QString sVendor,QWidget *wnd )
{
	emit SetDeviceByVendorSignal(sVendor, wnd);

	m_QSubviewProcess->SetCreateDeviceFlags(false);
	while(!m_QSubviewProcess->GetCreateDeviceFlags()){
		msleep(10);
	}
	return m_QSubviewProcess->GetDeviceClient();
	
}
