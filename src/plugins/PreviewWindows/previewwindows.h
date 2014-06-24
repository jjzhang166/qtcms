#ifndef PREVIEWWINDOWS_H
#define PREVIEWWINDOWS_H

#include "previewwindows_global.h"
#include "IWebPluginBase.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include "IDisplayWindowsManager.h"
#include <QtGui/QtGui>
#include "qpreviewwindows.h"
#include "qpreviewwindowsex.h"

class PreviewWindows :public QWidget
	,public IWebPluginBase
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

	virtual void nextPage();

	virtual void prePage();

	virtual int getCurrentPage();

	virtual int getPages();

	virtual int setDivMode( QString divModeName );

	virtual QString getCureentDivMode();
private:
	int						m_nRef;
	QMutex					m_csRef;
	/*QPreviewWindows			m_PreviewWindows;*/
	qpreviewwindowsex			m_PreviewWindows;
};

#endif // PREVIEWWINDOWS_H
