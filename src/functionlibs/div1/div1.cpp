#include "div1.h"
#include <guid.h>

div1::div1() :
m_nRef(0),
m_subWindows(NULL),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0)
{

}

div1::~div1()
{

}

void div1::setSubWindows( QWidget * windows,int count )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
}

void div1::setParentWindow( QWidget * parent )
{
	m_parentOfSubWindows = parent;
}

void div1::flush()
{
	int i;
	for (i = 0; i < m_nSubWindowCount; i ++)
	{
		if (i == m_nCurrentPage)
		{
			m_subWindows[i].show();
		}
		else
		{
			m_subWindows[i].hide();
		}
	}
}

void div1::parentWindowResize( QResizeEvent *ev )
{
	m_subWindows[m_nCurrentPage].resize(ev->size());
}

void div1::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
	qDebug("hi ha %s",subWindow->windowTitle().toAscii().data());
	return;
}

void div1::nextPage()
{
	m_nCurrentPage ++;
	if (m_nCurrentPage >= m_nSubWindowCount)
	{
		m_nCurrentPage = 0;
	}

	flush();
}

void div1::prePage()
{
	m_nCurrentPage --;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = m_nSubWindowCount - 1;
	}

	flush();
}

int div1::getCurrentPage()
{
	return m_nCurrentPage;
}

int div1::getPages()
{
	return m_nSubWindowCount;
}

QString div1::getModeName()
{
	return QString("div1");
}

long __stdcall div1::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div1::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div1::Release()
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