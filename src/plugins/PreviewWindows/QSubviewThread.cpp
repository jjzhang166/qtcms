#include "QSubviewThread.h"


QSubviewThread::QSubviewThread(void):m_IDeviceClient(NULL)
{
}


QSubviewThread::~QSubviewThread(void)
{
	if (NULL!=m_IDeviceClient)
	{
		m_IDeviceClient->Release();
		m_IDeviceClient=NULL;
	}
}

int QSubviewThread::SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
	m_DevCliSetInfo.m_sAddress.clear();
	m_DevCliSetInfo.m_sEseeId.clear();
	m_DevCliSetInfo.m_sUsername.clear();
	m_DevCliSetInfo.m_sPassword.clear();
	m_DevCliSetInfo.m_sCameraname.clear();
	m_DevCliSetInfo.m_sVendor.clear();
	m_DevCliSetInfo.m_sAddress=sAddress;
	m_DevCliSetInfo.m_sCameraname=sCameraname;
	m_DevCliSetInfo.m_sEseeId=sEseeId;
	m_DevCliSetInfo.m_sPassword=sPassword;
	m_DevCliSetInfo.m_sUsername=sUsername;
	m_DevCliSetInfo.m_sVendor=sVendor;
	m_DevCliSetInfo.m_uiChannelId=uiChannelId;
	m_DevCliSetInfo.m_uiPort=uiPort;
	m_DevCliSetInfo.m_uiStreamId=uiStreamId;
	return 0;
}

void QSubviewThread::OpenCameraInWnd()
{
	if (NULL==m_IDeviceClient)
	{
		return;
	}
	//申请接口使用
	IDeviceClient * m_IDeviceClientOpenCameraInWnd=NULL;
	m_IDeviceClient->QueryInterface(IID_IDeviceClient,(void**)&m_IDeviceClientOpenCameraInWnd);
	if (1!=m_IDeviceClientOpenCameraInWnd->setChannelName(m_DevCliSetInfo.m_sCameraname))
	{
		if (1!=m_IDeviceClientOpenCameraInWnd->connectToDevice(m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sEseeId))
		{
			if (1!=m_IDeviceClientOpenCameraInWnd->liveStreamRequire(m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,true));
			{
				m_IDeviceClientOpenCameraInWnd->Release();
				m_IDeviceClientOpenCameraInWnd=NULL;
				return;
			}
		}
	}
	m_IDeviceClientOpenCameraInWnd->Release();
	m_IDeviceClientOpenCameraInWnd=NULL;
	//if (1==m_IDeviceClient->setChannelName(m_DevCliSetInfo.m_sCameraname))
	//{
	//	return;
	//}
	//int nRet=1;
	//nRet=m_IDeviceClient->connectToDevice(m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sEseeId);
	//if (1==nRet)
	//{
	//	return;
	//}
	//if (1==m_IDeviceClient->liveStreamRequire(m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,true))
	//{
	//	return;
	//}
	//return;
}

int QSubviewThread::SetDeviceClient(IDeviceClient *parm)
{
	if (NULL==parm)
	{
		return 1;
	}
	if (NULL!=m_IDeviceClient)
	{
		m_IDeviceClient->Release();
		m_IDeviceClient=NULL;
	}
	parm->QueryInterface(IID_IDeviceClient,(void**)&m_IDeviceClient);
	if (NULL==m_IDeviceClient)
	{
		return 1;
	}
	return 0;
}
void QSubviewThread::CloseAll()
{
	if (NULL==m_IDeviceClient)
	{
		return;
	}
	//申请接口
	IDeviceClient *m_IDeviceClientCloseAll=NULL;
	m_IDeviceClient->QueryInterface(IID_IDeviceClient,(void**)&m_IDeviceClientCloseAll);
	m_IDeviceClientCloseAll->closeAll();
	m_IDeviceClientCloseAll->Release();
	m_IDeviceClientCloseAll=NULL;
	m_IDeviceClient->Release();
	m_IDeviceClient=NULL;
	return;
}