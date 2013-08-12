#include "viewwndplugin.h"

ViewWndPlugin::ViewWndPlugin()
{

}

ViewWndPlugin::~ViewWndPlugin()
{

}

QList<QWebPluginFactory::Plugin> ViewWndPlugin::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-preview-window");
	mimeType.description=QString("cms preview window");

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

QObject * ViewWndPlugin::create( const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues )
{
	QTextEdit * edit= new QTextEdit();
	edit->setObjectName(QString("a new plugin for test"));
	edit->setPlainText(QString("hello plugins"));
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return edit;
}

Q_EXPORT_PLUGIN2("ViewWndPlugin.dll", ViewWndPlugin)