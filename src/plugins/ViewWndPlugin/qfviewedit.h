#ifndef QFVIEWEDIT_H
#define QFVIEWEDIT_H

#include <QtGui/QTextEdit>
#include <QtWebKit/QWebElement>
#include <qwfw.h>
#include <QtCore/QStringList>
#include <QtCore/QVariantList>

class qfviewedit : public QTextEdit,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	qfviewedit(QWidget *parent = 0);
	~qfviewedit();

	typedef struct _tagValRet{
		int na;
		int nb;
	}ValRet;

public slots:
	void PrintText(QString sText);
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
	QStringList TestStringList( const QString & nTest);
	void RefTest(int & nTest);
	QStringList PointReturnTest();
	QVariantMap RetTest();
	



};

#endif // QFVIEWEDIT_H
