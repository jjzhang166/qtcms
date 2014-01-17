#ifndef _QTWEBVIWE_11222_H_
#define _QTWEBVIWE_11222_H_

#include <QDebug>
#include <qwfw.h>
#include "SubWebView.h"
#include <QList>
#include <QDomDocument>
#include <QWidget>
#include <QObject>


class QtQWebView:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	QtQWebView();
	~QtQWebView();

typedef struct _tagViewPage{
	QString url;
	SubWebView *m_SubWebView;
}tagViewPage;
	public slots:
		void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

		void LoadNewPage(QString url);

		void LoadNewPageFromViewSignal(const QString &text);
private:
	QList<tagViewPage> m_ViewPageList;
};

#endif



