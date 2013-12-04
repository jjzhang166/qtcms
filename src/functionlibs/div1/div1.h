#ifndef DIV1_H
#define DIV1_H

#include "div1_global.h"
#include <QtCore/QObject>
#include <QList>
#include <QtCore/QMutex>
#include <IWindowDivMode.h>

class div1 : public QObject,
	public IWindowDivMode
{
public:
	div1();
	~div1();

//	virtual void setSubWindows( QWidget * windows,int count );
	virtual void setSubWindows( QList<QWidget *> windows,int count );

	virtual void setParentWindow( QWidget * parent );

	virtual void flush();

	virtual void parentWindowResize( QResizeEvent *ev );

	virtual void subWindowDblClick( QWidget *subWindow,QMouseEvent * ev );

	virtual void nextPage();

	virtual void prePage();

	virtual int getCurrentPage();

	virtual int getPages();

	virtual QString getModeName();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();
private:
	int m_nRef;
	QMutex m_csRef;
//	QWidget * m_subWindows;
	QList<QWidget *> m_subWindows; 
	int m_nSubWindowCount;
	QWidget * m_parentOfSubWindows;
	int m_nCurrentPage;
};

#endif // DIV1_H
