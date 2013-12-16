#include "qfviewedit.h"
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>
#include <QtCore/QCryptographicHash>

qfviewedit::qfviewedit(QWidget *parent)
	: QTextEdit(parent),
	QWebPluginFWBase(this)
{
}

qfviewedit::~qfviewedit()
{
	m_mapEventProc.clear();
	QVariantMap mml;
}

void qfviewedit::PrintText( QString sText )
{
	qDebug("%s",sText.toAscii().data());
	DEF_EVENT_PARAM(t);
	EP_ADD_PARAM(t,"hello","hi");
	EP_ADD_PARAM(t,"text","this is a test");
	EventProcCall("Passed",t);
}

QStringList qfviewedit::TestStringList(const QString & sTest)
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

void qfviewedit::RefTest( int & nTest )
{
	nTest = 10;
}

QStringList qfviewedit::PointReturnTest()
{
	QStringList a;
	a.insert(0,"1");
	a.insert(1,"3");
	QByteArray s = QCryptographicHash::hash(QByteArray("hello"),QCryptographicHash::Md5);
	a.insert(2,QString(s.toHex().toLower()));
	return a;
}

QVariantMap qfviewedit::RetTest()
{
	QVariantMap ret;
	ret.insert("name",QVariant("admin"));
	ret.insert("password",QVariant("abc"));
	ret.insert("address",QVariant("192.168.2.12"));
	ret.insert("port",QVariant(80));

	return ret;
}
int qfviewedit::my_test(){
	int nRet =0;
	return nRet;
}