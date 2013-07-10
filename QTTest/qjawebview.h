#ifndef QJAWEBVIEW_H
#define QJAWEBVIEW_H

#include <QWebView>

class QJaWebView : public QWebView
{
    Q_OBJECT
public:
    explicit QJaWebView(QWidget *parent = 0);

private:
    QString m_sApplicationPath;

protected:
    virtual void keyPressEvent(QKeyEvent *ev);

signals:
    
public slots:
    
};

#endif // QJAWEBVIEW_H
