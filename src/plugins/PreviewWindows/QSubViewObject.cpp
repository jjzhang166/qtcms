#include "QSubViewObject.h"


QSubViewObject::QSubViewObject(void):m_IDeviceClient(NULL),
	m_QSubviewProcess(NULL),
	bIsOpen(false)
{
	m_QSubviewProcess=new QSubviewThread;
	m_QSubviewProcess->moveToThread(&m_workerThread);
	connect(&m_workerThread, SIGNAL(finished()), m_QSubviewProcess, SLOT(deleteLater()));
	connect(this,SIGNAL(OpenCameraInWndSignl()),m_QSubviewProcess,SLOT(OpenCameraInWnd()),Qt::QueuedConnection);
	connect(this,SIGNAL(m_workerThreadQuitSignal()),this,SLOT(m_workerThreadQuit()),Qt::QueuedConnection);
	connect(this,SIGNAL(SetDeviceByVendorSignal(QString, QWidget *)),m_QSubviewProcess,SLOT(SetDeviceByVendor(QString, QWidget *)),Qt::QueuedConnection);

	m_workerThread.start();
}


QSubViewObject::~QSubViewObject(void)
{
	/*CloseWndCamera();*/
	if (bIsOpen)
	{
		CloseWndCamera();
	}
	
	m_workerThread.quit();
	m_workerThread.wait();
}
int QSubViewObject::OpenCameraInWnd()
{
	bIsOpen=true;
	emit OpenCameraInWndSignl();
	return 0;
}
int QSubViewObject::CloseWndCamera()
 {
	 if (bIsOpen&&m_QSubviewProcess->isFinished()==false)
	 {
		  QFuture<void>ret=QtConcurrent::run(m_QSubviewProcess,&QSubviewThread::CloseAll);
	 }
	
	bIsOpen=false;
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
		msleep(100);
	}
	return m_QSubviewProcess->GetDeviceClient();
	
}
