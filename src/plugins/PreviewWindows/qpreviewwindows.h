#ifndef QPREVIEWWINDOWS_H
#define QPREVIEWWINDOWS_H

#include <QWidget>
#include "qsubview.h"
#include "qwfw.h"

class QPreviewWindows : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	QPreviewWindows(QWidget *parent = 0);
	~QPreviewWindows();

	virtual void resizeEvent( QResizeEvent * );

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};
private:
	QSubView * m_PreviewWnd[4];
};

#endif // QPREVIEWWINDOWS_H
