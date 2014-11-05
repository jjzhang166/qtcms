#ifndef div4_4_H
#define div4_4_H

#include "div4_4_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <IWindowDivMode.h>

class div4_4 : public QObject,
	public IWindowDivMode
{
public:

#define PAGE_SUBCOUNT 16
	div4_4();
	~div4_4();

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

	QList<QWidget *> m_subWindows; 
	int m_nSubWindowCount;          //总的子窗口数目
	QWidget * m_parentOfSubWindows; //当前大窗口的指针
	int m_nCurrentPage;             //当前页页码值
	int m_nTotalWindowsCount;       //页面总数

	int m_nColumn;                  //一列的子窗口数
	int m_nRow;                     //一行的子窗口数
	bool m_bIsMax;                  //是否已被双击放大
};

#endif // div4_4_H
