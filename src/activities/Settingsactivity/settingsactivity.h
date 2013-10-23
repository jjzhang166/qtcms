#ifndef TESTACTIVITY_H
#define TESTACTIVITY_H

#include "settingsactivity_global.h"
#include <IActivities.h>
#include <qwfw.h>
#include <IUserManager.h>
#include <IDeviceManager.h>
#include <IAreaManager.h>
#include <QMutex>
#include <QObject>
#include <QPoint>
#include <QtGui/QMessageBox>
class settingsActivity : public QWebUiFWBase,
	public IActivities
{
	Q_OBJECT
public:
	settingsActivity();

public:
	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual void Active( QWebFrame * frame);

public slots:
	void OnTopActDbClick();
	void OnMouseDown();
	void OnMouseUp();
	void OnMouseMove();
	void OnMaxClick();
	void OnMinClick();
	void OnCloseClick();
	void OnAddUserOk();
	void OnModifyUserOk();
	void OnDeleteUserOk();
	
	/*device module*/
	void OnAddDevice();
	void OnRemoveDevice();
	void OnModifyDevice();


private:
	QWidget * m_MainView;
	int m_nRef;
	QMutex m_csRef;
 	bool m_bMouseTrace;
	QPoint m_pos;


};

#endif // TESTACTIVITY_H
