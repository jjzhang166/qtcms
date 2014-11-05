#ifndef DIV2_2_H
#define DIV2_2_H

#include "div2_2_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <IWindowDivMode.h>
#include <QtWebKit/QWebFrame>
#include <QtGui/QSplitter>

class div2_2 : public QObject,
	public IWindowDivMode
{

public:
	div2_2();
	~div2_2();

	virtual void setSubWindows( QList<QWidget *> windows,int count );

	virtual void setParentWindow( QWidget * parent );

	virtual void flush();

	virtual void parentWindowResize( QResizeEvent *ev );


	virtual void nextPage();

	virtual void prePage();

	virtual int getCurrentPage();

	virtual int getPages();

	virtual QString getModeName();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();

	virtual void subWindowDblClick( QWidget *subWindow,QMouseEvent * ev );
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


#endif // DIV2_2_H
