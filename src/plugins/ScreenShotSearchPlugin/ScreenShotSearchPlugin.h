#ifndef SCREENSHOTSEARCHPLUGIN_H
#define SCREENSHOTSEARCHPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include "IWebPluginBase.h"
#include "screenshotsch.h"


class ScreenShotSearchPlugin : public QObject,
	public IWebPluginBase
{
	Q_OBJECT 
public:
	ScreenShotSearchPlugin();
	~ScreenShotSearchPlugin();

	virtual QList<QWebPluginFactory::Plugin> plugins() const ;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const ;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;
};

#endif // SCREENSHOTSEARCHPLUGIN_H
