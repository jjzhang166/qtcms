#ifndef QPREVIEWWINDOWS_H
#define QPREVIEWWINDOWS_H

#include <QWidget>
#include <QMutex>
#include "qsubview.h"
#include "qwfw.h"
#include "IWindowDivMode.h"

class QPreviewWindows : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	QPreviewWindows(QWidget *parent = 0);
	~QPreviewWindows();

	virtual void resizeEvent( QResizeEvent * );

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};

	virtual void nextPage();

	virtual void prePage();

	virtual int getCurrentPage();

	virtual int getPages();

	virtual int setDivMode( QString divModeName );

	virtual QString getCureentDivMode();

	void OnSubWindowDblClick(QWidget *,QMouseEvent *);

	void SetCurrentWind(QWidget *);

	int GetCurrentWnd();

	int OpenCameraInWnd(unsigned int uiWndIndex
		,const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname
		,const QString & sVendor);

	int CloseWndCamera(unsigned int uiWndIndex);

	int GetWindowConnectionStatus(unsigned int uiWndIndex);

	void CurrentStateChangePlugin(int statevalue);

private:
	QSubView m_PreviewWnd[64];
	IWindowDivMode * m_DivMode;
	QList<QWidget *> m_PreviewWndList;

	int m_uiWndIndex;
	volatile int m_CurrentWnd;

	QMutex m_mutex;

};

#endif // QPREVIEWWINDOWS_H
