#pragma once
#include <IActivities.h>
#include <QtWebKit/QtWebKit>

class autoSearchDeviceWindow: public QWebView
{
	 Q_OBJECT
public:
	explicit autoSearchDeviceWindow(QWidget *parent = 0);
	~autoSearchDeviceWindow();
public:
	void loadHtmlUrl(QString sUrl);
public slots:
	void cancel();
	void OnLoad(bool bOk);
	void OnurlChanged(const QUrl & url);
protected:
	//virtual void mouseMoveEvent ( QMouseEvent * event ) ;
	//virtual void mousePressEvent(QMouseEvent *event);
	//virtual void mouseReleaseEvent(QMouseEvent *event);
signals:
	void sgCancel();
private:
	IActivities * m_tActivity;
	QString m_sApplicationPath;
	QPoint m_tWindowPos;
	QPoint m_tMousePos;
	QPoint m_tDPos;
	bool m_bMove;
};

