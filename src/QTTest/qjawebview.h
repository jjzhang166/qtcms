#ifndef QJAWEBVIEW_H
#define QJAWEBVIEW_H

#include <QtWebKit/QtWebKit>

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


};

#endif // QJAWEBVIEW_H
