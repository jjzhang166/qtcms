#ifndef DIV8_8_H
#define DIV8_8_H

#include "div8_8_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <IWindowDivMode.h>

class div8_8 : public QObject,
	public IWindowDivMode
{
public:
	div8_8();
	~div8_8();

	virtual void setSubWindows(  QList<QWidget *> windows,int count );

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

	virtual void ChangePosition();
private:
	int m_nRef;
	QMutex m_csRef;
//	QWidget * m_subWindows;
	QList<QWidget *> m_subWindows; 
	int m_nSubWindowCount;
	QWidget * m_parentOfSubWindows;
	int m_nCurrentPage;

	int m_nCloum;
	int m_nRow;
	int m_nWindowsPerPage;
	int m_nWidth;
	int m_nHeight;
	int m_nCurrentWindow;

	QSize m_parentSize;
};

#endif // DIV8_8_H
