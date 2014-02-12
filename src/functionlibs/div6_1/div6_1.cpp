#include "div6_1.h"
#include <guid.h>

div6_1::div6_1() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),
m_nRow(3),
m_nCloum(3),
m_bIsLastPage(false),
m_nIndexPerPage(0)
{
	m_nWindowsPerPage = PAGESFORWINDOW;
}

div6_1::~div6_1()
{

}

void div6_1::setSubWindows( QList<QWidget *> windows,int count )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
}

void div6_1::setParentWindow( QWidget * parent )
{
	m_parentOfSubWindows = parent;
}

void div6_1::ChangePosition()
{
	int index = 0;
	m_nWidth = m_parentOfSubWindows->width()/m_nCloum;
	m_nHeight = m_parentOfSubWindows->height()/m_nRow;

	while ( index < m_nSubWindowCount )
	{
 		if (m_bIsLastPage && index >= m_nIndexPerPage)//最后一页
 		{
 			index = m_nIndexPerPage;
 		}
		for (int i = 0; i < m_nRow; i++)
		{
			for (int j = 0; j < m_nCloum; j++)
			{
				if (!(index%PAGESFORWINDOW) && index/PAGESFORWINDOW < m_nSubWindowCount/PAGESFORWINDOW || m_bIsLastPage && index == m_nIndexPerPage)
				{
					m_subWindows.at(index)->move(0,0);
					m_subWindows.at(index++)->resize(2*m_nWidth, 2*m_nHeight);
					continue;
				}
				if ( i >= m_nRow-1 || j >= m_nCloum-1 )
				{
					m_subWindows.at(index)->move(j*m_nWidth,i*m_nHeight);
					m_subWindows.at(index++)->resize(m_nWidth,m_nHeight);
				}
				if ( index >= m_nSubWindowCount )
				{
					break;
				}
			}
			if ( index >= m_nSubWindowCount )
			{
				break;
			}
		}
	}
}

void div6_1::flush()
{
	int i = 0;
	bool Conditions = false;
	ChangePosition();

	for (i = 0; i < m_nSubWindowCount; i++)
	{
		Conditions = (i >= m_nCurrentPage*m_nWindowsPerPage) && (i < (m_nCurrentPage + 1)* m_nWindowsPerPage);//没放大的时的判断条件
		if (m_bIsLastPage && m_nWindowsPerPage > 1)
		{
			Conditions = (i >= m_nIndexPerPage) && (i < m_nIndexPerPage + PAGESFORWINDOW);
		}

		if ( Conditions )
		{
			if (1 == m_nWindowsPerPage)
			{
				//放大该窗口
				m_subWindows.at(m_nCurrentPage)->move(0,0);
				m_subWindows.at(m_nCurrentPage)->resize(m_nWidth*m_nCloum,m_nHeight*m_nRow);
			}
			m_subWindows.at(i)->show();
		}
		else
		{
			m_subWindows.at(i)->hide();
		}
	}
}

void div6_1::parentWindowResize( QResizeEvent *ev )
{
	ChangePosition();
}

void div6_1::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
	if ( m_nWindowsPerPage > 1)
	{
		m_nWindowsPerPage = 1;
		for (int i = 0; i < m_nSubWindowCount; i++)
		{
			if (m_subWindows.at(i) == subWindow)
			{
				m_nCurrentPage = i;
				break;
			}
		}
	}
	else
	{
		m_nWindowsPerPage = PAGESFORWINDOW;
		m_nCurrentPage = m_bIsLastPage?m_nSubWindowCount/m_nWindowsPerPage:m_nCurrentPage/m_nWindowsPerPage;
	}

	flush();
}

void div6_1::nextPage()
{
	int Pages = getPages();
	Pages = (m_nWindowsPerPage > 1)?(Pages - 1):Pages;
	m_nCurrentPage++;
	//防止超出最大页数
	if ( (m_nCurrentPage >= Pages && 1 == m_nWindowsPerPage) || (m_nCurrentPage > Pages && m_nWindowsPerPage > 1) )
	{
		m_nCurrentPage = 0;
	}

	if ( m_nCurrentPage == m_nSubWindowCount/PAGESFORWINDOW || ( (1 == m_nWindowsPerPage) && (m_nCurrentPage > m_nSubWindowCount - PAGESFORWINDOW) ))
	{
		m_nIndexPerPage = m_nSubWindowCount - PAGESFORWINDOW;
		m_bIsLastPage = true;
	}
	else
	{
		m_nIndexPerPage = m_nCurrentPage*PAGESFORWINDOW;
		m_bIsLastPage = false;
	}

	flush();
}

void div6_1::prePage()
{
	int Pages = getPages();
	Pages = (m_nWindowsPerPage > 1)?(Pages - 1):Pages;
	m_nCurrentPage--;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = (m_nWindowsPerPage > 1)?Pages:Pages -1;
	}
	if ( m_nCurrentPage == m_nSubWindowCount/PAGESFORWINDOW || ( (1 == m_nWindowsPerPage) && (m_nCurrentPage > m_nSubWindowCount - PAGESFORWINDOW) ))
	{
		m_nIndexPerPage = m_nSubWindowCount - PAGESFORWINDOW;
		m_bIsLastPage = true;
	}
	else
	{
		m_nIndexPerPage = m_nCurrentPage*PAGESFORWINDOW;
		m_bIsLastPage = false;
	}

	flush();
}

int div6_1::getCurrentPage()
{
	return m_nCurrentPage;
}

int div6_1::getPages()
{
	int Pages = 0;
	if (m_nWindowsPerPage > 1)//没放大
	{
		Pages = m_nSubWindowCount/(PAGESFORWINDOW) + 1;
	}
	else
	{
		Pages = m_nSubWindowCount;
	}
	return Pages;
}

QString div6_1::getModeName()
{
	return QString("div6_1");
}

long __stdcall div6_1::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWindowDivMode == iid)
	{
		*ppv = static_cast<IWindowDivMode *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall div6_1::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div6_1::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}