#include "qpreviewactivity.h"
#include <QWebPage>

QPreviewActivity::QPreviewActivity(QObject *parent) :
    QObject(parent)
{
}

void QPreviewActivity::Active( QWebFrame *Frame )
{
	qDebug("this");
	m_Frame = Frame;
	connect(m_Frame,SIGNAL(javaScriptWindowObjectCleared),this,SLOT(OnJavaScriptWindowObjectCleared));
	m_Frame->addToJavaScriptWindowObject(QString("qob"),this);

	//×¢²áÏûÏ¢º¯Êý
	m_Frame->evaluateJavaScript(QString("connectEvent('top_act','dblclick',function a(){qob.OnTopActDbClick();});"));
}

void QPreviewActivity::OnJavaScriptWindowObjectCleared()
{
	m_Frame->addToJavaScriptWindowObject(QString("qob"),this);
}

void QPreviewActivity::OnTopActDbClick()
{
	QWebPage * pages = m_Frame->page();
	QWidget *view = pages->view();
	view->showNormal();
}