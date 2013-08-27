#ifndef QFVIEWEDIT_H
#define QFVIEWEDIT_H

#include <QtGui/QTextEdit>
#include "ui_qfviewedit.h"
#include <QtWebKit/QWebElement>
#include <qwfw.h>

class qfviewedit : public QTextEdit,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	qfviewedit(QWidget *parent = 0);
	~qfviewedit();

private:
 	Ui::qfviewedit ui;

public slots:
	void PrintText(const QString & sText);
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}


};

#endif // QFVIEWEDIT_H
