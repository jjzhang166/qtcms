#ifndef PREVIEWACTIVITY_H
#define PREVIEWACTIVITY_H

#include "previewactivity_global.h"
#include <IActivities.h>
#include <QObject>

class previewactivity : public QObject,
	public IActivities
{
	Q_OBJECT
public:
    virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();

	virtual void Active( QWebFrame * frame);
private:
	QWebFrame * m_MainFrame;
	QWebView * m_MainView;

};

#endif // PREVIEWACTIVITY_H
