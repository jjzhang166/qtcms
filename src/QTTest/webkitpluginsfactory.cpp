#include "webkitpluginsfactory.h"
#include <QCoreApplication>
#include <QDir>
#include <QtXml/QtXml>
#include <libpcom.h>
#include <IWebPluginBase.h>
#include <guid.h>

WebkitPluginsFactory::WebkitPluginsFactory(QObject *parent)
	: QWebPluginFactory(parent)
{

}

WebkitPluginsFactory::~WebkitPluginsFactory()
{

}

QList<QWebPluginFactory::Plugin> WebkitPluginsFactory::plugins() const
{
	static bool bFirst = true;
	static QList<QWebPluginFactory::Plugin> plugins;
	if (!bFirst)
	{
		return plugins;
	}
	bFirst = false;

	// clear list first
	plugins.clear();
	
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
	for (n = 0;n < itemList.count(); n ++)
	{
		QDomNode item;
		item = itemList.at(n);
		QDomElement itemElement = item.toElement();
		QString sItemName = itemElement.attribute("name");

		// Get the node named as "plugin."
		if (sItemName.left(strlen("plugin.")) == QString("plugin."))
		{
			QWebPluginFactory::MimeType mimeType;
			mimeType.name = itemElement.attribute("mimetype");
			mimeType.description=sItemName;

			QWebPluginFactory::Plugin pluginTemp;
			pluginTemp.name = sItemName;
			pluginTemp.description = sItemName;
			pluginTemp.mimeTypes.append(mimeType);

			plugins.insert(plugins.size(),pluginTemp);
		}
	}
	file->close();
	delete file;
	file=NULL;
	return plugins;
}

QObject * WebkitPluginsFactory::create( const QString& mimeType, const QUrl& url, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
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
	for (n = 0; n < itemList.count();n++)
	{
		QDomNode item = itemList.at(n);
		QDomElement itemElement = item.toElement();
		QString sItemName = itemElement.attribute("name");

		// Get the node named as "plugin."
		if (sItemName.left(strlen("plugin.")) == QString("plugin."))
		{
			QString sMime = itemElement.attribute("mimetype");
			if (sMime.toLower() == mimeType)
			{
				// Get item classID
				QString sItemClsid = itemElement.attribute("clsid");
				GUID guidTemp = pcomString2GUID(sItemClsid);

				// append plugins
				IWebPluginBase * webPluginBase = NULL;
				pcomCreateInstance(guidTemp,NULL,IID_IWebPluginBase,(void **)&webPluginBase);
				qDebug("create plugin :%s",sItemName.toAscii().data());
				if (NULL != webPluginBase)
				{
					QObject * retObj = webPluginBase->create(mimeType,url,argumentNames,argumentValues);
					webPluginBase->Release();
					file->close();
					delete file;
					file=NULL;
					return retObj;
				}
			}
		}
	}
	file->close();
	delete file;
	file=NULL;
	return NULL;
}