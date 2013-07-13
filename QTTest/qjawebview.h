#ifndef QJAWEBVIEW_H
#define QJAWEBVIEW_H

#include <QWebView>

class QJaWebView : public QWebView
{
    Q_OBJECT
public:
    explicit QJaWebView(QWidget *parent = 0);
    void aTest();

private:
    QString m_sApplicationPath;

protected:
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual bool eventFilter(QObject *, QEvent *);

signals:
    
public slots:
    void OnLoadFinished(bool bOk);
    
};

#endif // QJAWEBVIEW_H
