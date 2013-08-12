#ifndef WEBKITPLUGINSFACTORY_H
#define WEBKITPLUGINSFACTORY_H

#include <QWebPluginFactory>

class WebkitPluginsFactory : public QWebPluginFactory
{
	Q_OBJECT

public:
	WebkitPluginsFactory(QObject *parent);
	~WebkitPluginsFactory();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;

	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;
private:
	// 插件列表

	mutable QList<QList<QWebPluginFactory::Plugin> > pluginslist;

	//插件接口，这个接口是我们自定义的插件的同意接口。

	//这个接口在后面会讲到。

	mutable QList<WebKitPluginInterface *> interfaces;
};

#endif // WEBKITPLUGINSFACTORY_H
