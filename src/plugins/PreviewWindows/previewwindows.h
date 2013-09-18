#ifndef PREVIEWWINDOWS_H
#define PREVIEWWINDOWS_H

#include "previewwindows_global.h"
#include "IWebPluginBase.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>

class PreviewWindows :public QObject,
	public IWebPluginBase
{
	Q_OBJECT
public:
	PreviewWindows();
	~PreviewWindows();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;

	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();
private:
	int						m_nRef;
	QMutex					m_csRef;

};

#endif // PREVIEWWINDOWS_H
