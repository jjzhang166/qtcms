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
	QWebPluginFWBase(this)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(m_PreviewWnd); i ++)
	{
		m_PreviewWnd[i].setParent(this);
		connect(&m_PreviewWnd[i],SIGNAL(mouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(OnSubWindowDblClick(QWidget *,QMouseEvent *)));
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
				m_DivMode->setSubWindows(m_PreviewWnd,ARRAY_SIZE(m_PreviewWnd));
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
	m_DivMode->prePage();
}

int QPreviewWindows::getCurrentPage()
{
	return m_DivMode->getCurrentPage();
}

int QPreviewWindows::getPages()
{
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
					m_DivMode->setSubWindows(m_PreviewWnd,ARRAY_SIZE(m_PreviewWnd));
					m_DivMode->flush();
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
	return m_DivMode->getModeName();
}

void QPreviewWindows::OnSubWindowDblClick( QWidget * wind,QMouseEvent * ev)
{
	m_DivMode->subWindowDblClick(wind,ev);
}

int QPreviewWindows::GetCurrentWnd( unsigned int uiWndIndex )
{

}

int QPreviewWindows::OpenCameraInWnd( unsigned int uiWndIndex ,const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sVendor )
{

}

int QPreviewWindows::CloseWndCamera( unsigned int uiWndIndex )
{

}

int QPreviewWindows::GetWindowConnectionStatus( unsigned int uiWndIndex )
{

}