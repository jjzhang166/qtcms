#include "div8_8.h"
#include <guid.h>

div8_8::div8_8() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),
m_nRow(8),
m_nCloum(8),
m_nCurrentWindow(0)
{
	m_nWindowsPerPage = m_nCloum * m_nRow;
}

div8_8::~div8_8()
{

}

void div8_8::setSubWindows( QList<QWidget *> windows,int count )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
}

void div8_8::setParentWindow( QWidget * parent )
{
	m_parentSize = parent->size();
	m_parentOfSubWindows = parent;
}

void div8_8::ChangePosition()
{
	int index = 0;
 	m_nWidth = m_parentOfSubWindows->width()/m_nCloum;
 	m_nHeight = m_parentOfSubWindows->height()/m_nRow;
	while ( index < m_nSubWindowCount )
	{
		for (int i = 0; i < m_nRow; i++)
		{
			for (int j = 0; j < m_nCloum; j++)
			{
				m_subWindows.at(index)->move(j*m_nWidth,i*m_nHeight);
				m_subWindows.at(index++)->resize(m_nWidth,m_nHeight);
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

void div8_8::flush()
{
	int i;

	ChangePosition();
	for (i = 0; i < m_nSubWindowCount; i ++)
	{
		if ( i >= m_nCurrentPage*m_nWindowsPerPage && i < (m_nCurrentPage + 1)*m_nWindowsPerPage )
		{
			if (1 == m_nWindowsPerPage)
			{
				m_subWindows.at(i)->move(0,0);
				m_subWindows.at(i)->resize(m_nWidth*m_nCloum,m_nHeight*m_nRow);
			}
			m_subWindows.at(i)->show();
		}
		else
		{
			m_subWindows.at(i)->hide();
		}
	}
}

void div8_8::parentWindowResize( QResizeEvent *ev )
{
	ChangePosition();
}

void div8_8::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
	if ( m_nWindowsPerPage > 1)
	{
		m_nWindowsPerPage = 1;
		for (int i = 0; i < m_nSubWindowCount; i++)
		{
			if (m_subWindows.at(i)== subWindow)
			{
				m_nCurrentWindow = i;
				m_nCurrentPage = m_nCurrentWindow;
				break;
			}
		}
	}
	else
	{
		m_nWindowsPerPage = m_nCloum * m_nRow;
		m_nCurrentPage = m_nSubWindowCount/m_nWindowsPerPage -1;
	}

	flush();
}

void div8_8::nextPage()
{
	int Pages = getPages();
	m_nCurrentPage = getCurrentPage();

	m_nCurrentPage++;
	if (m_nCurrentPage >= Pages)
	{
		m_nCurrentPage = 0;
	}

	flush();
}

void div8_8::prePage()
{
	int Pages = getPages();
	m_nCurrentPage = getCurrentPage();

	m_nCurrentPage--;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = Pages - 1;
	}

	flush();
}

int div8_8::getCurrentPage()
{
	return m_nCurrentPage;
}

int div8_8::getPages()
{
	int Pages = 0;
	if (m_nWindowsPerPage > 1)//Ã»·Å´ó
	{
		Pages = m_nSubWindowCount/m_nWindowsPerPage;
	}
	else
	{
		Pages = m_nSubWindowCount;
	}
	return Pages;
}

QString div8_8::getModeName()
{
	return QString("div8_8");
}

long __stdcall div8_8::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div8_8::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div8_8::Release()
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