#ifndef QJAWEBVIEW_H
#define QJAWEBVIEW_H

#include <QtWebKit/QtWebKit>
#include <IActivities.h>
#include <QWebInspector>

//#define __USE_WEB_DEBUGER__

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
#ifdef __USE_WEB_DEBUGER__
	QWebInspector m_webinspector;
#endif
};

#endif // QJAWEBVIEW_H
