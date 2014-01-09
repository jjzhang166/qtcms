#include "previewwindowstest.h"
#include <guid.h>
#include <QtWebKit/QWebPluginFactory>
#include <QtPlugin>
#include <libpcom.h>
#include "PreviewWndTest.h"

PreviewWindowTest::PreviewWindowTest():
m_nRef(0)
{

}

PreviewWindowTest::~PreviewWindowTest()
{

}


QList<QWebPluginFactory::Plugin> PreviewWindowTest::plugins() const
{
    QWebPluginFactory::MimeType mimeType;
    mimeType.name = QString("application/cms-manualrecord-test");
    mimeType.description=QString("cms manualrecord test");

    QList<QWebPluginFactory::MimeType> mimeTypes;
    mimeTypes.append(mimeType);

    QWebPluginFactory::Plugin plugin;
    plugin.name = QString("cms manualrecord test");
    plugin.description = QString("cms manualrecord test");
    plugin.mimeTypes=mimeTypes;

    QList<QWebPluginFactory::Plugin> plugins;
    plugins.append(plugin);
    return plugins;
}

QObject * PreviewWindowTest::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
    PreviewWndTest * pPreviewWndTest= new PreviewWndTest();
    Q_UNUSED(argumentNames);
    Q_UNUSED(argumentValues);
    return pPreviewWndTest;
}

long __stdcall PreviewWindowTest::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall PreviewWindowTest::AddRef()
{
    m_csRef.lock();
    m_nRef ++;
    m_csRef.unlock();
    return m_nRef;
}

unsigned long __stdcall PreviewWindowTest::Release()
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

Q_EXPORT_PLUGIN2("PreviewWindowTest.dll",PreviewWindowTest)