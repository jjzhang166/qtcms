#include "qfviewedit.h"
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>

qfviewedit::qfviewedit(QWidget *parent)
	: QTextEdit(parent),
	QWebPluginFWBase(this)
{
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

QStringList qfviewedit::TestStringList()
{
	QStringList ret;
	ret.insert(0,QString("This is a string list,it's the 1st line"));
	ret.insert(1,QString("Here test the 2nd line"));
	ret.insert(2,QString("And more..."));
	ret.insert(3,QString("Test finished,it's the last line"));
	ret.insert(4,QString("Oh,wait.I forgot one thing."));
	ret.insert(5,QString("Thank you for your test"));

	return ret;
}