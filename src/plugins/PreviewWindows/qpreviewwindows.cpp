#include "qpreviewwindows.h"
#include "qsubview.h"
#include <QtGui/QResizeEvent>
#include <qwfw_tools.h>
#include <QtCore/QtCore>
#include <QtXml/QtXml>
#include <libpcom.h>
#include <guid.h>
#include "IDisplayWindowsManager.h"

QPreviewWindows::QPreviewWindows(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this),
	m_uiWndIndex(0),
	m_CurrentWnd(0)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(m_PreviewWnd); i ++)
	{
		m_PreviewWnd[i].setParent(this);
		connect(&m_PreviewWnd[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_PreviewWnd[i],SIGNAL(mousePressEvent(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowRmousePress(QWidget *,QMouseEvent *)));
		connect(&m_PreviewWnd[i],SIGNAL(SetCurrentWindSignl(QWidget *)),this,SLOT(SetCurrentWind(QWidget *)));
		connect(&m_PreviewWnd[i],SIGNAL(CurrentStateChangeSignl(int,QWidget *)),this,SLOT(CurrentStateChangePlugin(int,QWidget *)));
		m_PreviewWndList.insert(m_PreviewWndList.size(),&m_PreviewWnd[i]);
	}

	// 读取配置文件，将第一个读到的divmode作为默认分割方式
	m_DivMode = NULL;
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");

		if (sItemName.left(strlen("divmode.")) == QString("divmode."))
		{
			if (NULL != m_DivMode)
			{
				m_DivMode->Release();
				m_DivMode = NULL;
			}
			CLSID divModeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(divModeClsid,NULL,IID_IWindowDivMode,(void **)&m_DivMode);
			if (NULL != m_DivMode)
			{
				m_DivMode->setParentWindow(this);
				m_DivMode->setSubWindows(m_PreviewWndList,ARRAY_SIZE(m_PreviewWnd));
				m_DivMode->flush();
			}
			break;
		}
	}

	file->close();
	delete file;

	// connect signals

}

QPreviewWindows::~QPreviewWindows()
{
	if (NULL != m_DivMode)
	{
		m_DivMode->Release();
		m_DivMode = NULL;
	}
}

void QPreviewWindows::resizeEvent( QResizeEvent * ev)
{
	if (NULL != m_DivMode)
	{
		m_DivMode->parentWindowResize(ev);
	}
}

void QPreviewWindows::nextPage()
{
	if (NULL != m_DivMode)
	{
		m_DivMode->nextPage();
	}
}

void QPreviewWindows::prePage()
{
	if (NULL==m_DivMode)
	{
		return;
	}
	m_DivMode->prePage();
}

int QPreviewWindows::getCurrentPage()
{
	if (NULL==m_DivMode)
	{
		return -1;
	}
	return m_DivMode->getCurrentPage();
}

int QPreviewWindows::getPages()
{
	if (NULL==m_DivMode)
	{
		return -1;
	}
	return m_DivMode->getPages();
}

int QPreviewWindows::setDivMode( QString divModeName )
{
	// configuration
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	bool bFound = false;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");

		if (sItemName.left(strlen("divmode.")) == QString("divmode."))
		{
			QString sDivMode = item.toElement().attribute("mode");
			if (divModeName == sDivMode)
			{
				bFound = true;
				if (NULL != m_DivMode)
				{
					m_DivMode->Release();
					m_DivMode = NULL;
				}
				CLSID divModeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
				pcomCreateInstance(divModeClsid,NULL,IID_IWindowDivMode,(void **)&m_DivMode);
				if (NULL != m_DivMode)
				{
					m_DivMode->setParentWindow(this);
					m_DivMode->setSubWindows(m_PreviewWndList,ARRAY_SIZE(m_PreviewWnd));
					m_DivMode->flush();
					m_CurrentWnd=0;
					m_uiWndIndex=0;
				}
			}
		}
	}

	file->close();
	delete file;

	if (!bFound)
	{
		return IDisplayWindowsManager::E_MODE_NOT_SUPPORT;
	}

	return IDisplayWindowsManager::OK;
}

QString QPreviewWindows::getCureentDivMode()
{
	if (NULL==m_DivMode)
	{
		QString s_Return;
		return s_Return;
	}
	return m_DivMode->getModeName();
}

void QPreviewWindows::OnSubWindowDblClick( QWidget * wind,QMouseEvent * ev)
{
	if (NULL==m_DivMode)
	{
		return ;
	}
	m_DivMode->subWindowDblClick(wind,ev);
}

void QPreviewWindows::SetCurrentWind(QWidget *wind)
{
	int j;
	for (j=0;j<ARRAY_SIZE(m_PreviewWnd);j++)
	{
		if (&m_PreviewWnd[j]==wind)
		{
			break;
		}
	}
	m_mutex.lock();
	m_CurrentWnd=j;
	m_mutex.unlock();
	return ;
}

int QPreviewWindows::GetCurrentWnd()
{
	return m_CurrentWnd;
}

int QPreviewWindows::OpenCameraInWnd( unsigned int uiWndIndex ,const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor )
{
	if (uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_PreviewWnd))
	{
		return 1;
	}
	m_mutex.lock();
	m_CurrentWnd=uiWndIndex;
	m_mutex.unlock();
	m_PreviewWnd[uiWndIndex].OpenCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	return 0;
}
int QPreviewWindows::SetCameraInWnd(unsigned int uiWndIndex ,const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname ,const QString & sVendor)
{
	if (uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_PreviewWnd))
	{
		return 1;
	}
	m_mutex.lock();
	m_CurrentWnd=uiWndIndex;
	m_mutex.unlock();
	m_PreviewWnd[uiWndIndex].SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	return 0;
}
int QPreviewWindows::CloseWndCamera( unsigned int uiWndIndex )
{
	if (uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_PreviewWnd))
	{
		return 1;
	}
	m_mutex.lock();
	m_CurrentWnd=uiWndIndex;
	m_mutex.unlock();
	m_PreviewWnd[uiWndIndex].CloseWndCamera();
	return 0;
}

int QPreviewWindows::GetWindowConnectionStatus( unsigned int uiWndIndex )
{
	if (uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_PreviewWnd))
	{
		return 0;
	}
	m_mutex.lock();
	m_CurrentWnd=uiWndIndex;
	m_mutex.unlock();
	return m_PreviewWnd[uiWndIndex].GetWindowConnectionStatus();
}

void QPreviewWindows::CurrentStateChangePlugin(int statevalue,QWidget *WID)
{
	int j;
	for (j=0;j<ARRAY_SIZE(m_PreviewWnd);j++)
	{
		if (&m_PreviewWnd[j]==WID)
		{
			break;
		}
	}
	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"CurrentState",statevalue);
	EP_ADD_PARAM(arg,"WPageId",j);
	EventProcCall("CurrentStateChange",arg);
	return ;
}

void QPreviewWindows::OnSubWindowRmousePress( QWidget *Wid,QMouseEvent *ev )
{
	int j;
	for (j=0;j<ARRAY_SIZE(m_PreviewWnd);j++)
	{
		if (&m_PreviewWnd[j]==Wid)
		{
			break;
		}
	}
	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"Wid",j);
	EventProcCall("CurrentWindows",arg);
}

int QPreviewWindows::StartRecord()
{
	int nRet = 0;
	nRet = m_PreviewWnd[m_CurrentWnd].StartRecord();

	return nRet;
}

int QPreviewWindows::StopRecord()
{
	int nRet = 0;
	nRet = m_PreviewWnd[m_CurrentWnd].StopRecord();

	return nRet;
}

int QPreviewWindows::SetDevInfo(const QString&devname,int nChannelNum)
{
	int nRet = 0;
	nRet = m_PreviewWnd[m_CurrentWnd].SetDevInfo(devname, nChannelNum);

	return nRet;	
}