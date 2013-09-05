#include "ccommonlib.h"
#include <guid.h>
#include <QtPlugin>
#include "qcommonplugin.h"

Ccommonlib::Ccommonlib() :
m_nRef(0),
m_pluginObj(NULL)
{

}

Ccommonlib::~Ccommonlib()
{

}

QList<QWebPluginFactory::Plugin> Ccommonlib::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-common-library");
	mimeType.description=QString("cms common functions");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms common functions");
	plugin.description = QString("cms common functions");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * Ccommonlib::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	QCommonPlugin * obj = new QCommonPlugin(NULL);
	return obj;
}

long __stdcall Ccommonlib::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWebPluginBase == iid)
	{
		*ppv = static_cast<IWebPluginBase *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IUserManager == iid)
	{
		*ppv = static_cast<IUserManager *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall Ccommonlib::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall Ccommonlib::Release()
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

int Ccommonlib::AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 )
{
	return m_pluginObj.AddUser(sUsername,sPassword,nLevel,nAuthorityMask1,nAuthorityMask2);
}

int Ccommonlib::RemoveUser( const QString & sUsername )
{
	return m_pluginObj.RemoveUser(sUsername);
}

int Ccommonlib::ModifyUserPassword( const QString & sUsername,const QString & sNewPassword )
{
	return m_pluginObj.ModifyUserPassword(sUsername,sNewPassword);
}

int Ccommonlib::ModifyUserLevel( const QString & sUsername,int nLevel )
{
	return m_pluginObj.ModifyUserLevel(sUsername,nLevel);
}

int Ccommonlib::ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 )
{
	return m_pluginObj.ModifyUserAuthorityMask(sUsername,nAuthorityMask1,nAuthorityMask2);
}

bool Ccommonlib::IsUserExists( const QString & sUsername )
{
	return m_pluginObj.IsUserExists(sUsername);
}

bool Ccommonlib::CheckUser( const QString & sUsername,const QString & sPassword )
{
	return m_pluginObj.CheckUser(sUsername,sPassword);
}

int Ccommonlib::GetUserLevel( const QString & sUsername,int & nLevel )
{
	return m_pluginObj.GetUserLevel(sUsername,nLevel);
}

int Ccommonlib::GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 )
{
	return m_pluginObj.GetUserAuthorityMask(sUsername,nAuthorityMask1,nAuthorityMask2);
}

int Ccommonlib::GetUserCount()
{
	throw std::exception("The method or operation is not implemented.");
}

QStringList Ccommonlib::GetUserList()
{
	throw std::exception("The method or operation is not implemented.");
}

Q_EXPORT_PLUGIN2("commonlib.dll",Ccommonlib)