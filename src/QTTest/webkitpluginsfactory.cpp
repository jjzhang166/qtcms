#include "webkitpluginsfactory.h"
#include <QCoreApplication>
#include <QDir>

WebkitPluginsFactory::WebkitPluginsFactory(QObject *parent)
	: QWebPluginFactory(parent)
{

}

WebkitPluginsFactory::~WebkitPluginsFactory()
{

}

QList<QWebPluginFactory::Plugin> WebkitPluginsFactory::plugins() const
{
	// plugin directory ${ApplicationPath}/plugins
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sPluginPath = sAppPath + "/plugins";

	// Check if the first time
	static bool isFirst=true;
	static QList<QWebPluginFactory::Plugin> plugins;
	if(!isFirst)
	{
		return plugins;
	}
	isFirst=false;

	// Clear at first
	plugins.clear();


	QDir dir(sPluginPath);
	QStringList filters;
	QString abspath=dir.absolutePath();
	//获取指定目录下的所有插件，linux下是插件库的后缀为so，windows下则是dll，
	filters<<"lib*.so";
	QStringList files=dir.entryList(filters);
	foreach(QString file,files)
	{

		file=dir.filePath(file);

		QPluginLoader loader(file);
		QObject * obj= loader.instance();

		//下面是载入自定义的接口，只有这样才能支持动态插件创建，如果固定死了，将不利于扩展
		WebKitPluginInterface * interface= qobject_cast<WebKitPluginInterface*> (obj);
		if(interface==0)
		{

			continue;
		}
		plugins.append(interface->plugins());
		pluginslist.append(interface->plugins());
		interfaces.append(interface);
	}
	if(plugins.isEmpty())
	{
		qDebug()<<"no plugins is loaded!";
	}
	return plugins;
}

QObject * WebkitPluginsFactory::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	for(int i=0;i<pluginslist.size();i++)
	{
		for( int j=0;j< pluginslist[i].size();j++)
		{
			foreach(QWebPluginFactory::MimeType mt, pluginslist[i][j].mimeTypes)
			{
				if(mt.name == mimeType) //更具MIME类型，创建相应的插件实例
					return interfaces[i]-> create( mimeType, url, argumentNames, argumentValues);
			}
		}
	}
	return NULL; //如果没有，直接返回NULL，webkit会进行处理的
}