#include "qpreviewactivity.h"
#include <QWebPage>

void MessageEvent(QWebFrame * frame,QString sId,QString sEvent,QString sAck)
{
    QString sEventMap = "connectEvent('"+sId+"','"+sEvent+"',function a(){qob."+sAck+"();});";
    frame->evaluateJavaScript(sEventMap);
}


QPreviewActivity::QPreviewActivity(QObject *parent) :
    QObject(parent)
{
}

void QPreviewActivity::Active( QWebFrame *Frame )
{
    qDebug("this2");
	m_Frame = Frame;
	connect(m_Frame,SIGNAL(javaScriptWindowObjectCleared),this,SLOT(OnJavaScriptWindowObjectCleared));
	m_Frame->addToJavaScriptWindowObject(QString("qob"),this);

    MessageEvent(m_Frame,"top_act","dblclick","OnTopActDbClick");
    MessageEvent(m_Frame,"WindowClose","click","OnWindowClose");
}

void QPreviewActivity::OnJavaScriptWindowObjectCleared()
{
	m_Frame->addToJavaScriptWindowObject(QString("qob"),this);
}

void QPreviewActivity::OnTopActDbClick()
{
    QWidget * view = m_Frame->page()->view();
    if (view->isMaximized())
    {
        view->showNormal();
        int nWidth = view->width();
        int nHeight = view->height();
        qDebug("width:%d height:%d",nWidth,nHeight);
        if (nWidth < 1024) nWidth = 1024;
        if (nHeight < 768) nHeight = 768;
        view->resize(nWidth,nHeight);
        view->move(0,0);
    }
    else view->showMaximized();
}

void QPreviewActivity::OnWindowClose()
{
    m_Frame->page()->view()->close();
}
