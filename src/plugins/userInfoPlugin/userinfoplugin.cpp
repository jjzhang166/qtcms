#include "userinfoplugin.h"
#include <guid.h>
userInfoPlugin::userInfoPlugin():m_nRef(0)
{

}

userInfoPlugin::~userInfoPlugin()
{

}

QList<QWebPluginFactory::Plugin> userInfoPlugin::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-local-userInfo");
	mimeType.description=QString("cms local userInfo");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms local userInfo");
	plugin.description = QString("cms local userInfo");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * userInfoPlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	userInfoObject * pUserInfoObject= new userInfoObject();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);

	return pUserInfoObject;
}

long __stdcall userInfoPlugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall userInfoPlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall userInfoPlugin::Release()
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
