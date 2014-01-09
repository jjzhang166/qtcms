#ifndef PREVIEWWINDOWSPLUGINTEST_H
#define PREVIEWWINDOWSPLUGINTEST_H

#include "previewwindowstest_global.h"
#include <QObject>
#include <QMutex>
#include "IWebPluginBase.h"


class PreviewWindowTest :public QObject
    ,public IWebPluginBase
{
    Q_OBJECT
public:
    PreviewWindowTest();
    ~PreviewWindowTest();
    virtual QList<QWebPluginFactory::Plugin> plugins() const;
    virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;
    virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
private:
    int						m_nRef;
    QMutex					m_csRef;

};

#endif // PREVIEWWINDOWSPLUGINTEST_H
