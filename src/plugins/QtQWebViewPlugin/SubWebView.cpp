#include "SubWebView.h"
#include <webkitpluginsfactory.h>
#include <guid.h>
#include <QtXml/QtXml>
#include <libpcom.h>
#include <QSettings>

SubWebView::SubWebView(QString nurl,QWidget *parent):QWebView(parent),
	m_url(nurl),
	m_Activity(NULL)
{
	// Window styles
	setWindowFlags(Qt::FramelessWindowHint);

	// Set object name
	setObjectName("QtWebKitFW");

	// Enable Javascript and plugins
	settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
	settings()->setAttribute(QWebSettings::PluginsEnabled,true);

	// Enable Local content access remote url
	settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);

	// Disable context menu
	setContextMenuPolicy(Qt::NoContextMenu);

	// Set web plugin factory
	page()->setPluginFactory(new WebkitPluginsFactory(this));

	// Connect Signals
	connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoad(bool)));
	connect(this,SIGNAL(urlChanged(const QUrl &)),this,SLOT(OnurlChanged(const QUrl &)));
	connect(this,SIGNAL(statusBarMessage(const QString &)),this,SLOT(OnstatusBarMessage(const QString &)));

	QString temp=QCoreApplication::applicationDirPath();
	m_sApplicationPath.append(temp);
	QString sUiDir=m_sApplicationPath.append(m_url);
	load("file:///" + sUiDir);
}


SubWebView::~SubWebView(void)
{
	if (NULL != m_Activity)
	{
		m_Activity->Release();
	}
	setObjectName("NULL");
}

void SubWebView::OnLoad( bool bOk )
{
	if (bOk)
	{
		// test for widget levels

		// end of test



		QWebFrame * MainFrame;
		MainFrame = page()->mainFrame();

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

					if (NULL != m_Activity)
					{
						m_Activity->Release();
						m_Activity = NULL;
					}
					pcomCreateInstance(guidTemp,NULL,IID_IActivities,(void **)&m_Activity);
					if (NULL != m_Activity)
					{
						MainFrame->setScrollBarPolicy(Qt::Horizontal,Qt::ScrollBarAlwaysOff);
						MainFrame->setScrollBarPolicy(Qt::Vertical,Qt::ScrollBarAlwaysOff);
						m_Activity->Active(MainFrame);
					}
				}
			}
		}
		file->close();
	}
}

void SubWebView::OnstatusBarMessage( const QString &text )
{
	qDebug()<<text;
	emit LoadOrChangeUrl(text);
}

void SubWebView::OnurlChanged( const QUrl & url )
{
	Q_UNUSED(url);
	if (NULL != m_Activity)
	{
		m_Activity->Release();
		m_Activity = NULL;
	}
}

void SubWebView::keyPressEvent( QKeyEvent* ev )
{
	switch(ev->key()){
	case  Qt::Key_Escape:{
		close();
						 }
						 break;
	}
	QWebView::keyPressEvent(ev);
}
