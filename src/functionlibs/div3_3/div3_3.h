#ifndef DIV3_3_H
#define DIV3_3_H

#include "div3_3_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <IWindowDivMode.h>

class div3_3 : public QObject,
	public IWindowDivMode
{
public:
	div3_3();
	~div3_3();

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

	virtual void ChangePosition();
private:
	int m_nRef;
	QMutex m_csRef;
//	QWidget * m_subWindows;
	QList<QWidget *> m_subWindows; 
	int m_nSubWindowCount;
	QWidget * m_parentOfSubWindows;
	int m_nCurrentPage;

	int m_nCloum;//列
	int m_nRow;//行
	int m_nWindowsPerPage;//每页的窗口数
	int m_nWidth;//列宽
	int m_nHeight;//行高
	int m_nIndexPerPage;//每页第一个窗口的索引
	bool m_bIsLastPage;//是否最后一页
};

#endif // DIV3_3_H
