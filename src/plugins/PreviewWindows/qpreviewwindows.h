#ifndef QPREVIEWWINDOWS_H
#define QPREVIEWWINDOWS_H

#include <QWidget>
#include "qsubview.h"
#include "qwfw.h"
#include "IWindowDivMode.h"

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

	virtual void nextPage();

	virtual void prePage();

	virtual int getCurrentPage();

	virtual int getPages();

	virtual int setDivMode( QString divModeName );
private:
	QSubView m_PreviewWnd[64];
	IWindowDivMode * m_DivMode;

};

#endif // QPREVIEWWINDOWS_H
