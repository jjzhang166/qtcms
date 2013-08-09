#ifndef PREVIEWACTIVITY_H
#define PREVIEWACTIVITY_H

#include "previewactivity_global.h"
#include <IActivities.h>
#include <QObject>
#include <QMutex>
#include <qwfw.h>

class previewactivity : public QObject,
	public IActivities,
	public QWebUiFWBase
{
	Q_OBJECT
public:
	previewactivity();
public:
    virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();

	virtual void Active( QWebFrame * frame);

signals:

public slots:
	void OnJavaScriptWindowObjectCleared();
	void OnTopActDbClick();
	void OnTopActMouseDown();
	void OnTopActMouseUp();
	void OnTopActMouseMove(int x,int y);

private:
	QWidget * m_MainView;
	int m_nRef;
	QMutex m_csRef;

};

#endif // PREVIEWACTIVITY_H
