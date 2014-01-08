#include <QtCore/QList>
#include <QtWebKit/QWebPluginFactory>
#include <QtPlugin>
#include <QWSWindow>
#include <libpcom.h>
#include <guid.h>
#include "RecordPlayerPlugins.h"

RecordPlayerPlug::RecordPlayerPlug() :
m_nRef(0)
{
}

RecordPlayerPlug::~RecordPlayerPlug()
{

}

QList<QWebPluginFactory::Plugin> RecordPlayerPlug::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-record-player");
	mimeType.description=QString("cms record player");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms record player");
	plugin.description = QString("cms record player");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * RecordPlayerPlug::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	qDebug()<<"RecordPlayerPlug";
	RecordPlayer * pRecordPlayer = new RecordPlayer();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return pRecordPlayer;
}

long __stdcall RecordPlayerPlug::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall RecordPlayerPlug::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall RecordPlayerPlug::Release()
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

