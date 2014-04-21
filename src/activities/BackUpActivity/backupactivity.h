#ifndef BACKUPACTIVITY_H
#define BACKUPACTIVITY_H

#include "backupactivity_global.h"
#include <IActivities.h>
#include <QObject>
#include <QMutex>
#include <qwfw.h>
class  BackUpActivity: public QWebUiFWBase,
	public IActivities
{
	Q_OBJECT
public:
	BackUpActivity();
	~BackUpActivity();

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual void Active( QWebFrame * frame);
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

#endif // BACKUPACTIVITY_H
