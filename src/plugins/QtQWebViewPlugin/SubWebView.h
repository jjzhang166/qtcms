#ifndef _SUBWEBWIEW_HEADFILE_H_
#define _SUBWEBWIEW_HEADFILE_H_
#include <IActivities.h>

#include <QtWebKit/QtWebKit>
class SubWebView:public QWebView
{
	Q_OBJECT
public:
	explicit SubWebView(QString nurl,QWidget *parent = 0);
	virtual ~SubWebView(void);
protected:
	virtual void keyPressEvent(QKeyEvent* ev);
signals:
	void LoadOrChangeUrl(const QString &text);
	public slots:
		void OnLoad(bool bOk);
		void OnstatusBarMessage(const QString &text);
		void OnurlChanged(const QUrl & url);
private:
	QString m_sApplicationPath;
	QString m_url;
private:
	IActivities *m_Activity;

};


#endif
