#include "qjawebview.h"
#include <QCoreApplication>
#include <QSettings>
#include <QKeyEvent>
#include <QWebPage>
#include <QWebFrame>
#include <QtDebug>
#include <QWebHitTestResult>
#include <QWebElement>


QJaWebView::QJaWebView(QWidget *parent) :
    QWebView(parent)
{
    // connect signals
    connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoadFinished(bool)));

    // No frame window
    setWindowFlags(Qt::FramelessWindowHint);

    // Get Application Path
    QString temp = QCoreApplication::applicationDirPath();
    m_sApplicationPath.append(temp);

    // Read Main ini file
    QSettings MainIniFile(m_sApplicationPath + "/MainSet.ini",QSettings::IniFormat);
    QString sTheme = MainIniFile.value(QString("Configure/Theme")).toString();
    QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
    QString sUiDir = m_sApplicationPath + sThemeDir;
    qDebug("%s",sUiDir.toAscii().data());
    load("file:///" + sUiDir);

}

void QJaWebView::aTest()
{
    qDebug("aTest");
}

void QJaWebView::keyPressEvent(QKeyEvent *ev)
{
    switch(ev->key())
    {
    case Qt::Key_Escape:
        {
            close();
        }
        break;
    }
}

bool QJaWebView::eventFilter(QObject *object, QEvent *event)
{
    if( QEvent::MouseButtonRelease == event->type() )
    {
        qDebug() << "Mouse release";
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if(mouseEvent->button() == Qt::LeftButton)
        {
            QWebView * view = dynamic_cast<QWebView *>(object);

            QPoint pos = view->mapFromGlobal(mouseEvent->globalPos());
            QWebFrame * frame = view->page()->frameAt(mouseEvent->pos());
            if (frame != NULL)
            {
                QWebHitTestResult rs = frame->hitTestContent(pos);
                qDebug() << "element" << rs.element().localName();
            }
        }
    }
    return false;
}

void QJaWebView::OnLoadFinished(bool bOk)
{
    if(bOk)
    {
        QWebPage * pPage = page();
        QWebFrame * pMainFrame = pPage->mainFrame();
        QString sFrameTitle = pMainFrame->title();
        if(sFrameTitle == QString("preview"))
        {
            pMainFrame->installEventFilter(this);
        }
    }
}
