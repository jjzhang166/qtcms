#ifndef WEBKITPLUGINSFACTORY_H
#define WEBKITPLUGINSFACTORY_H

#include <QWebPluginFactory>

class WebkitPluginsFactory : public QWebPluginFactory
{
	Q_OBJECT

public:
	WebkitPluginsFactory(QObject *parent);
	~WebkitPluginsFactory();

	virtual QList<Plugin> plugins() const;

	virtual QObject * create( const QString& mimeType, const QUrl& url, const QStringList& argumentNames, const QStringList& argumentValues ) const;
};

#endif // WEBKITPLUGINSFACTORY_H
