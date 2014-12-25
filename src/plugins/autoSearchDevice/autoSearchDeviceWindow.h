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
	void cancelLoginUI();
	void OnLoad(bool bOk);
	void OnurlChanged(const QUrl & url);
protected:
	 bool eventFilter(QObject *obj, QEvent *ev);
signals:
	void sgCancel();
	void sgCancelLoginUI();
private:
	IActivities * m_tActivity;
	QString m_sApplicationPath;
	QPoint m_tWindowPos;
	QPoint m_tMousePos;
	QPoint m_tDPos;
	bool m_bMove;
};

