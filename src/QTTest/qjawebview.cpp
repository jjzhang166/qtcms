#include "qjawebview.h"
#include <libpcom.h>
#include <QCoreApplication>
#include <QSettings>
#include <QKeyEvent>

#include <QtXml/QtXml>
#include <guid.h>
#include <webkitpluginsfactory.h>


QJaWebView::QJaWebView(QWidget *parent) :
    QWebView(parent),
	m_Activity(NULL)
{
	// Window styles
    setWindowFlags(Qt::FramelessWindowHint);

	// Set object name
	setObjectName("QtWebKitFW");

	// Enable Javascript and plugins
	settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
	settings()->setAttribute(QWebSettings::PluginsEnabled,true);
	settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows,true);
	settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows,true);

	// Enable Local content access remote url
	settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);

	// Disable context menu
	setContextMenuPolicy(Qt::NoContextMenu);
#ifdef __USE_WEB_DEBUGER__DUMP__
	QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
#ifdef WIN32
	Dumper::setVersionInfo("cms_1.1.13_09_20_10", strlen("cms_1.1.13_09_20_10"));
	m_pdup = new Dumper();
#endif
#endif

	// Set web plugin factory
	page()->setPluginFactory(new WebkitPluginsFactory(this));


    // Get Application Path
    QString temp = QCoreApplication::applicationDirPath();

    m_sApplicationPath.append(temp);

	// Connect Signals
    connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoad(bool)));
	connect(this,SIGNAL(urlChanged(const QUrl &)),this,SLOT(OnurlChanged(const QUrl &)));
	connect(this,SIGNAL(statusBarMessage(const QString &)),this,SLOT(OnstatusBarMessage(const QString &)));

    // Read Main ini file
    QSettings MainIniFile(m_sApplicationPath + "/MainSet.ini",QSettings::IniFormat);
    QString sTheme = MainIniFile.value(QString("Configure/Theme")).toString();
    QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
    QString sUiDir = m_sApplicationPath + sThemeDir;
    qDebug("%s",sUiDir.toAscii().data());
    load("file:///" + sUiDir);

}

QJaWebView::QJaWebView( bool bLoadDefault, QWidget *parent /*= 0*/):
	QWebView(parent),
	m_Activity(NULL)
{
	// Window styles
	setWindowFlags(Qt::FramelessWindowHint);

	// Set object name
	setObjectName("QtWebKitFW");

	// Enable Javascript and plugins
	settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
	settings()->setAttribute(QWebSettings::PluginsEnabled,true);
	settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows,true);
	settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows,true);

	// Enable Local content access remote url
	settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);

	// Disable context menu
	//setContextMenuPolicy(Qt::NoContextMenu);

	// Set web plugin factory
	page()->setPluginFactory(new WebkitPluginsFactory(this));


	// Get Application Path
	QString temp = QCoreApplication::applicationDirPath();

	m_sApplicationPath.append(temp);

	// Connect Signals
	connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoad(bool)));
	connect(this,SIGNAL(urlChanged(const QUrl &)),this,SLOT(OnurlChanged(const QUrl &)));

	if (bLoadDefault)
	{
		// Read Main ini file
		QSettings MainIniFile(m_sApplicationPath + "/MainSet.ini",QSettings::IniFormat);
		QString sTheme = MainIniFile.value(QString("Configure/Theme")).toString();
		QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
		QString sUiDir = m_sApplicationPath + sThemeDir;
		qDebug("%s",sUiDir.toAscii().data());
		load("file:///" + sUiDir);
	}
}
QJaWebView::~QJaWebView()
{
	if (NULL != m_Activity)
	{
		m_Activity->Release();
	}
	setObjectName("NULL");
#ifdef __USE_WEB_DEBUGER__DUMP__
#ifdef WIN32
	if (m_pdup)
	{
		delete m_pdup;
	}
#endif
#endif
}

void QJaWebView::keyPressEvent(QKeyEvent *ev)
{
    switch(ev->key())
    {
    case Qt::Key_Escape:
        {
           /* close();*/
			//qDebug()<<__FUNCTION__<<__LINE__<<"CLOSE CMS FROM Key_Escape";
        }
        break;
#ifdef __USE_WEB_DEBUGER__DUMP__
	case Qt::Key_F12:
		{
			m_webinspector.setPage(page());
			m_webinspector.show();
			int mheight=m_webinspector.height();
			int mwidth=this->width();
			m_webinspector.resize(mwidth,mheight);
		}
		break;
	case Qt::Key_F5:
		{
			reload();
		}
		break;
#endif
	}
    QWebView::keyPressEvent(ev);
}

void QJaWebView::OnLoad( bool bOk )
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
		delete file;
		file=NULL;
    }
}

void QJaWebView::OnurlChanged( const QUrl & url )
{
	Q_UNUSED(url);
	if (NULL != m_Activity)
	{
		m_Activity->Release();
		m_Activity = NULL;
	}
}

void QJaWebView::OnstatusBarMessage( const QString &text )
{
    Q_UNUSED(text);
}

QWebView * QJaWebView::createWindow( QWebPage::WebWindowType type )
{
	QWebView * ret = new QJaWebView(false,NULL);
	ret->move(0,0);
	ret->showMaximized();
    Q_UNUSED(type);
	return ret;
}
