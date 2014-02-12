#ifndef PREVIEWACTIVITY_H
#define PREVIEWACTIVITY_H

#include "previewactivity_global.h"
#include <IActivities.h>
#include <QObject>
#include <QMutex>
#include <qwfw.h>

class previewactivity : public QWebUiFWBase,
	public IActivities
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
	void OnTopActDbClick();
	void OnCloseWindow();
	void OnMaxsizeWindow();
	void OnMinsizeWindow();

private:
	QWidget * m_MainView;
	int m_nRef;
	QMutex m_csRef;
	QWebFrame *m_frame;

};

#endif // PREVIEWACTIVITY_H
