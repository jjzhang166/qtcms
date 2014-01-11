#ifndef __IWEBPLUGINBASE_HEAD_FILE__
#define __IWEBPLUGINBASE_HEAD_FILE__

#include "libpcom.h"
#include <QtWebKit/QWebPluginFactory>

interface IWebPluginBase : public IPComBase
{
	virtual QList<QWebPluginFactory::Plugin> plugins() const = 0;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const = 0;
};

#endif