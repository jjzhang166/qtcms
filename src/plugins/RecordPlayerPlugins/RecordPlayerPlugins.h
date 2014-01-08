#ifndef __RECORDPLAYERPLUGIN_H__
#define __RECORDPLAYERPLUGIN_H__

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtGui/QtGui>
#include <QtCore/QDateTime>
#include "IWebPluginBase.h"
#include "RecordPlayer.h"

class RecordPlayerPlug :public QWidget
	,public IWebPluginBase
{
	Q_OBJECT
public:
	RecordPlayerPlug();
	~RecordPlayerPlug();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int						m_nRef;
	QMutex					m_csRef;
};

#endif // __RECORDPLAYERPLUGIN_H__
