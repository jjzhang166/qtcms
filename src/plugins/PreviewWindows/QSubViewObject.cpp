#include "QSubViewObject.h"


QSubViewObject::QSubViewObject(void):m_IDeviceClient(NULL),
	m_QSubviewProcess(NULL)
{
	m_QSubviewProcess=new QSubviewThread;
	m_QSubviewProcess->moveToThread(&m_workerThread);
	connect(&m_workerThread, SIGNAL(finished()), m_QSubviewProcess, SLOT(deleteLater()));
	connect(this,SIGNAL(OpenCameraInWndSignl()),m_QSubviewProcess,SLOT(OpenCameraInWnd()));
	connect(this,SIGNAL(CloseAllSignl()),m_QSubviewProcess,SLOT(CloseAll()));
	m_QSubviewProcess->SetDeviceClient(m_IDeviceClient);
	m_workerThread.start();
}


QSubViewObject::~QSubViewObject(void)
{
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
	emit CloseAllSignl();
	return 0;
}
int QSubViewObject::SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
	m_QSubviewProcess->SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	return 0;
}
int QSubViewObject::SetDeviceClient(IDeviceClient *m_IDeviceClient)
{
	m_QSubviewProcess->SetDeviceClient(m_IDeviceClient);
	return 0;
}