#include "SubWebView.h"
#include <webkitpluginsfactory.h>
#include <guid.h>
#include <QtXml/QtXml>
#include <libpcom.h>
#include <QSettings>

volatile bool SubWebView::bIsbuilding=false;
SubWebView::SubWebView(QString nurl,QSize mSize,QWidget *parent):QWebView(parent),
	m_url(nurl),
	IsLoad(false),
	m_Activity(NULL)
{
	// Window styles
	setWindowFlags(Qt::FramelessWindowHint);

	// Set object name
	setObjectName("QtWebKitFW");
	resize(mSize);
	// Enable Javascript and plugins
	settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
	settings()->setAttribute(QWebSettings::PluginsEnabled,true);

	// Enable Local content access remote url
	settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);

	// Disable context menu
	setContextMenuPolicy(Qt::NoContextMenu);
#ifdef __USE_WEB_DEBUGER__
	QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
#endif
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
	IsLoad=true;
}

void SubWebView::OnstatusBarMessage( const QString &text )
{
	statusBarMessage.clear();
	statusBarMessage.append(text);
	if (text.size()<5||bIsbuilding==true)
	{
		return;
	}
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
	case  Qt::Key_Escape:
		{
//			close();
			//hide();
			//emit CloseAllPage();
		}
		break;
#ifdef __USE_WEB_DEBUGER__
	case Qt::Key_F12:{
		m_webinspector.setPage(page());
		
		m_webinspector.show();
		int mheight=m_webinspector.height();
		int mwidth=this->width();
		m_webinspector.resize(mwidth,mheight);
					 }
					 break;
	case  Qt::Key_F5:{
		reload();
					 }
					 break;
#endif
	}
	QWebView::keyPressEvent(ev);
}

void SubWebView::OnRefressMessage()
{
	page()->mainFrame()->title();
	QVariantMap eventParam;
	eventParam.insert("title",page()->mainFrame()->title());
	eventParam.insert("refresh","true");
	eventParam.insert("Dsturl",m_url);
	QString sEvent="refresh";
	QString Scripte=EventProcsScripte(sEvent,eventParam);
	page()->mainFrame()->evaluateJavaScript(Scripte);
}

QString SubWebView::EventProcsScripte( QString sEvent,QVariantMap eventParam )
{
	QString sItem="subViewMsg(data)";
	QString sScripte;
	sScripte += "{var e={";
	QVariantMap::const_iterator itParameters;
	for (itParameters = eventParam.begin();itParameters != eventParam.end(); itParameters ++)
	{
		QString sKey = itParameters.key();
		QString sValue = itParameters.value().toString();
		sScripte += sKey;
		sScripte += ":'";
		sScripte += sValue;
		sScripte += "'";
		if (itParameters + 1 != eventParam.end())
		{
			sScripte += ",";
		}
	}
	QString sEventProc;
	sEventProc+=";var Proc={'Proc':'";
	sEventProc+=sEvent;
	sEventProc+="'};";
	sScripte += "}";
	sScripte+=sEventProc;
	sScripte += sItem.replace(QRegExp("\\((.*)\\)"),"(Proc,e)");
	sScripte += ";}";
	return sScripte;
}



