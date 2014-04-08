#include "QSubviewThread.h"
#include <QCoreApplication>
#include <QWidget>
#include <QFile>
#include <QtXml/QtXml>
#include "ISwitchStream.h"
#include <QtCore/QtCore>

QSubviewThread::QSubviewThread(void):m_IDeviceClient(NULL),
	m_bIsSysTime(false),
	m_bIsClosing(false)
{

}


QSubviewThread::~QSubviewThread(void)
{
	CloseAll();
	while(m_bIsClosing==true){
		msleep(100);
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

	if (NULL==m_IDeviceClient||m_bIsClosing)
	{
		return;
	}

	if ("JUAN IPC" == m_DevCliSetInfo.m_sVendor)
	{
		IAutoSycTime *pAutoSycTime = NULL;
		m_IDeviceClient->QueryInterface(IID_IAutoSycTime, (void**)&pAutoSycTime);
		if (NULL != pAutoSycTime)
		{
			pAutoSycTime->SetAutoSycTime(m_bIsSysTime);
			pAutoSycTime->Release();
		}
	}
	m_IDeviceClient->checkUser(m_DevCliSetInfo.m_sUsername, m_DevCliSetInfo.m_sPassword);

	//申请接口使用
	IDeviceClient * m_IDeviceClientOpenCameraInWnd=NULL;
	m_IDeviceClient->QueryInterface(IID_IDeviceClient,(void**)&m_IDeviceClientOpenCameraInWnd);
	if (1!=m_IDeviceClientOpenCameraInWnd->setChannelName(m_DevCliSetInfo.m_sCameraname))
	{
		if (1!=m_IDeviceClientOpenCameraInWnd->connectToDevice(m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sEseeId))
		{
            if (1!=m_IDeviceClientOpenCameraInWnd->liveStreamRequire(m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,true))
			{
				m_IDeviceClientOpenCameraInWnd->Release();
				m_IDeviceClientOpenCameraInWnd=NULL;
				return;
			}
		}
	}
	m_IDeviceClientOpenCameraInWnd->Release();
	m_IDeviceClientOpenCameraInWnd=NULL;
}


void QSubviewThread::CloseAll()
{
	//防止多线程同时调用
	if (m_bIsClosing==true)
	{
		return;
	}

	m_bIsClosing=true;
	if (NULL==m_IDeviceClient)
	{
		m_bIsClosing=false;
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
	m_bIsClosing=false;
	return;
}

int QSubviewThread::SetDeviceByVendor(QString sVendor, QWidget *pWnd)
{
	if (m_bIsClosing==false)
	{
		QString sAppPath=QCoreApplication::applicationDirPath();
		QFile *file=new QFile(sAppPath+"/pcom_config.xml");
		file->open(QIODevice::ReadOnly);
		QDomDocument ConFile;
		ConFile.setContent(file);
		QDomNode clsidNode=ConFile.elementsByTagName("CLSID").at(0);
		QDomNodeList itemList=clsidNode.childNodes();
		int n;
		for (n=0;n<itemList.count();n++)
		{
			QDomNode item=itemList.at(n);
			QString sItemName=item.toElement().attribute("vendor");
			if (sItemName==sVendor)
			{
				CLSID DeviceVendorClsid=pcomString2GUID(item.toElement().attribute("clsid"));
				if (NULL!=m_IDeviceClient)
				{
					m_IDeviceClient->Release();
					m_IDeviceClient=NULL;
				}
				pcomCreateInstance(DeviceVendorClsid,NULL,IID_IDeviceClient,(void**)&m_IDeviceClient);
				if (NULL!=m_IDeviceClient)
				{
					//设置主次码流
					if (pWnd->parentWidget()->width() == pWnd->width())
					{
						if (NULL!=m_IDeviceClient)
						{
							ISwitchStream *m_SwitchStream=NULL;
							m_IDeviceClient->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
							if (NULL!=m_SwitchStream)
							{
								m_SwitchStream->SwitchStream(0);
								m_SwitchStream->Release();
								m_SwitchStream=NULL;
							}
						}
					}
					else{
						if (NULL!=m_IDeviceClient)
						{
							ISwitchStream *m_SwitchStream=NULL;
							m_IDeviceClient->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
							if (NULL!=m_SwitchStream)
							{
								m_SwitchStream->SwitchStream(1);
								m_SwitchStream->Release();
								m_SwitchStream=NULL;
							}
						}
					}	
					bCreateDeviceFlags = true;
					return 0;
				}		
			}
		}
	}
	
	bCreateDeviceFlags = true;
	return 1;
}

IDeviceClient* QSubviewThread::GetDeviceClient()
{
	return m_IDeviceClient;
}

bool QSubviewThread::GetCreateDeviceFlags()
{
	return bCreateDeviceFlags;
}

void QSubviewThread::SetCreateDeviceFlags( bool flags )
{
	bCreateDeviceFlags = flags;
}
