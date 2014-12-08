#pragma once
#include <IActivities.h>
#include <QtWebKit/QtWebKit>

class autoSearchDeviceWindow: public QWebView
{
	 Q_OBJECT
public:
	explicit autoSearchDeviceWindow(QWidget *parent = 0);
	~autoSearchDeviceWindow();

public slots:
	void cancel();
	void OnLoad(bool bOk);
signals:
	void sgCancel();
private:
	IActivities * m_tActivity;
	QString m_sApplicationPath;
};

