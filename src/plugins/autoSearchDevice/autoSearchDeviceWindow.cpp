#include "autoSearchDeviceWindow.h"
#include <guid.h>
#include <webkitpluginsfactory.h>
#include <QtXml/QtXml>
autoSearchDeviceWindow::autoSearchDeviceWindow(QWidget *parent) :
QWebView(parent),
m_tActivity(NULL),
m_bMove(false)
{
	// Window styles
	setWindowFlags(Qt::FramelessWindowHint);

	// Set object name
	setObjectName("autoSearchDeviceWindow");

	// Enable Javascript and plugins
	settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
	settings()->setAttribute(QWebSettings::PluginsEnabled,true);
	settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows,true);
	settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows,true);

	
	// Enable Local content access remote url
	settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);
	// Disable context menu
	setContextMenuPolicy(Qt::NoContextMenu);
	// Set web plugin factory
	page()->setPluginFactory(new WebkitPluginsFactory(this));
	// Get Application Path
	QString temp = QCoreApplication::applicationDirPath();

	m_sApplicationPath.append(temp);

	// Connect Signals
	connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoad(bool)));

	// Read Main ini file
	QSettings MainIniFile(m_sApplicationPath + "/MainSet.ini",QSettings::IniFormat);
	QString sTheme = MainIniFile.value(QString("Configure/autoSearchDevice")).toString();
	QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
	QString sUiDir = m_sApplicationPath + sThemeDir;
	qDebug("%s",sUiDir.toAscii().data());
	load("file:///" + sUiDir);
}


autoSearchDeviceWindow::~autoSearchDeviceWindow(void)
{
}

void autoSearchDeviceWindow::OnLoad( bool bOk )
{
	if (bOk)
	{
		// test for widget levels

		// end of test



		QWebFrame * MainFrame;
		MainFrame = page()->mainFrame();
		MainFrame->addToJavaScriptWindowObject(QString("autoSearchDeviceModule"),this);
		//connect(MainFrame,SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(OnJavaScriptWindowObjectCleared())); 
		// Load configure file pcom_config.xml
		QString sAppPath = QCoreApplication::applicationDirPath();
		QFile *file = new QFile(sAppPath + "/pcom_config.xml");

		// use QDomDocument to analyse it
		QDomDocument ConfFile;
		ConfFile.setContent(file);

		// Get CLSID node,all object descripte under this node
		QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
		QDomNodeList itemList = clsidNode.childNodes();
		int n;
		for (n = 0;n < itemList.count();n ++)
		{
			QDomNode item;
			item = itemList.at(n);
			QString sItemName = item.toElement().attribute("name");
			// Get the node named like "activity."
			if (sItemName.left(strlen("activity.")) == QString("activity."))
			{
				// To fix the title
				QString sItemTitle = item.toElement().attribute("title");
				if (sItemTitle == MainFrame->title())
				{
					// find the activity and the make it work
					QString sItemClsid = item.toElement().attribute("clsid");
					GUID guidTemp = pcomString2GUID(sItemClsid);

					if (NULL != m_tActivity)
					{
						m_tActivity->Release();
						m_tActivity = NULL;
					}
					pcomCreateInstance(guidTemp,NULL,IID_IActivities,(void **)&m_tActivity);
					if (NULL != m_tActivity)
					{
						MainFrame->setScrollBarPolicy(Qt::Horizontal,Qt::ScrollBarAlwaysOff);
						MainFrame->setScrollBarPolicy(Qt::Vertical,Qt::ScrollBarAlwaysOff);
						m_tActivity->Active(MainFrame);
					}
				}
			}
		}
		file->close();
		delete file;
		file=NULL;
	}
}

void autoSearchDeviceWindow::cancel()
{
	emit sgCancel();
}

void autoSearchDeviceWindow::mouseMoveEvent( QMouseEvent * event )
{
	if (m_bMove)
	{
		this->move(event->globalPos()-this->m_tDPos);
	}
}

void autoSearchDeviceWindow::mousePressEvent( QMouseEvent *event )
{
	m_bMove=true;
	this->m_tWindowPos = this->pos();
	this->m_tMousePos = event->globalPos(); 
	this->m_tDPos=m_tMousePos-m_tWindowPos;
}

void autoSearchDeviceWindow::mouseReleaseEvent( QMouseEvent *event )
{
	m_bMove=false;
}
