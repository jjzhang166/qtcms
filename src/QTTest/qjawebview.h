#ifndef QJAWEBVIEW_H
#define QJAWEBVIEW_H

#include <QtWebKit/QtWebKit>
#include <IActivities.h>

class QJaWebView : public QWebView
{
    Q_OBJECT
public:
    explicit QJaWebView(QWidget *parent = 0);
	virtual ~QJaWebView();

private:
    QString m_sApplicationPath;

protected:
    virtual void keyPressEvent(QKeyEvent *ev);

signals:
    
public slots:
    void OnLoad(bool bOk);
private:
	IActivities * m_Activity;
};

#endif // QJAWEBVIEW_H
