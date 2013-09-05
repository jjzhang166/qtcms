#ifndef QFVIEWEDIT_H
#define QFVIEWEDIT_H

#include <QtGui/QTextEdit>
#include <QtWebKit/QWebElement>
#include <qwfw.h>
#include <QtCore/QStringList>

class qfviewedit : public QTextEdit,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	qfviewedit(QWidget *parent = 0);
	~qfviewedit();

public slots:
	void PrintText(const QString & sText);
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
	QStringList TestStringList();



};

#endif // QFVIEWEDIT_H
