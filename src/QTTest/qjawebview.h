#ifndef QJAWEBVIEW_H
#define QJAWEBVIEW_H

#include <QtWebKit/QtWebKit>
#include <IActivities.h>
#include <QWebInspector>

#ifdef WIN32
#include "minidumphead.h"
#pragma comment(lib, "minidump.lib")
#endif

#define __USE_WEB_DEBUGER__DUMP__

class QJaWebView : public QWebView
{
    Q_OBJECT
public:
    explicit QJaWebView(QWidget *parent = 0);
	QJaWebView(bool bLoadDefault, QWidget *parent = 0);
	virtual ~QJaWebView();

private:
    QString m_sApplicationPath;

protected:
    virtual void keyPressEvent(QKeyEvent *ev);

signals:
    
public slots:
    void OnLoad(bool bOk);
	void OnstatusBarMessage(const QString &text);
	void OnurlChanged(const QUrl & url);

	virtual QWebView * createWindow( QWebPage::WebWindowType type );
private:
	IActivities * m_Activity;
#ifdef __USE_WEB_DEBUGER__DUMP__
	QWebInspector m_webinspector;

#ifdef WIN32
	Dumper *m_pdup;
#endif
#endif
};

#endif // QJAWEBVIEW_H
