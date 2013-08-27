#include "qfviewedit.h"
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>

qfviewedit::qfviewedit(QWidget *parent)
	: QTextEdit(parent),
	QWebPluginFWBase(this)
{
	ui.setupUi(this);
}

qfviewedit::~qfviewedit()
{
	m_mapEventProc.clear();
}

void qfviewedit::PrintText( const QString & sText )
{
	qDebug("%s",sText.toAscii().data());
	EventProcCall("Passed");
}