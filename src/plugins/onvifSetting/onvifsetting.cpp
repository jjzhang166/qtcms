#include "onvifsetting.h"
#include <guid.h>
#include "onvifSettingObject.h"
#include <QtCore/QList>
#include <QtWebKit/QWebPluginFactory>
#include <QtPlugin>
onvifSetting::onvifSetting():
m_nRef(0)
{

}

onvifSetting::~onvifSetting()
{

}

QList<QWebPluginFactory::Plugin> onvifSetting::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name=QString("application/cms-onvif-setting");
	mimeType.description=QString("cms onvif setting");

	QList<QWebPluginFactory::MimeType>mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name=QString("cms onvif setting");
	plugin.description=QString("cms onvif setting");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * onvifSetting::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	onvifSettingObject *pOnvifSettingObject=new onvifSettingObject();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return pOnvifSettingObject;
}

long __stdcall onvifSetting::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWebPluginBase==iid)
	{
		*ppv=static_cast<IWebPluginBase*>(this);
	}else if (IID_IPcomBase==iid)
	{
		*ppv=static_cast<IPcomBase*>(this);
	}else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase*>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall onvifSetting::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall onvifSetting::Release()
{
	int nRet=0;
	m_csRef.lock();
	m_nRef--;
	nRet=m_nRef;
	m_csRef.unlock();
	if (0==nRet)
	{
		delete this;
	}
	return nRet;
}
