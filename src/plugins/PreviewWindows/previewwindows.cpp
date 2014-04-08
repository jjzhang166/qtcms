#include "previewwindows.h"
#include <QtCore/QList>
#include <QtWebKit/QWebPluginFactory>
#include <QtPlugin>
#include <QWSWindow>
#include <libpcom.h>
#include <guid.h>
#include "qpreviewwindows.h"

PreviewWindows::PreviewWindows() :
m_nRef(0)
{
	m_PreviewWindows.setParent(this);
}

PreviewWindows::~PreviewWindows()
{

}

QList<QWebPluginFactory::Plugin> PreviewWindows::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-preview-windows");
	mimeType.description=QString("cms preview windows");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms preview window");
	plugin.description = QString("cms preview window");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * PreviewWindows::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	//QWSWindow  *myQWSWindow =new QWSWindow();
	//myQWSWindow->Lowering();
	QPreviewWindows * previewWindows= new QPreviewWindows();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
    Q_UNUSED(mimeType);
	return previewWindows;
}

long __stdcall PreviewWindows::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWebPluginBase == iid)
	{
		*ppv = static_cast<IWebPluginBase *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall PreviewWindows::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall PreviewWindows::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

void PreviewWindows::nextPage()
{
	m_PreviewWindows.nextPage();
}

void PreviewWindows::prePage()
{
	m_PreviewWindows.prePage();
}

int PreviewWindows::getCurrentPage()
{
	return m_PreviewWindows.getCurrentPage();
}

int PreviewWindows::getPages()
{
	return m_PreviewWindows.getPages();
}

int PreviewWindows::setDivMode( QString divModeName )
{
	return m_PreviewWindows.setDivMode(divModeName);
}

QString PreviewWindows::getCureentDivMode()
{
	return m_PreviewWindows.getCureentDivMode();
}
Q_EXPORT_PLUGIN2("PreviewWindows.dll",PreviewWindows)