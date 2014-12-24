#include "div1.h"
#include <guid.h>

div1::div1() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0)
{

}

div1::~div1()
{

}

void div1::setSubWindows(  QList<QWidget *> windows,int count  )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;

	for (int i = 0; i < m_nSubWindowCount; i++)
	{
		m_subWindows.at(i)->resize(m_parentOfSubWindows->size());
	}
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
			m_subWindows.at(i)->setGeometry(0,0,m_parentOfSubWindows->width(),m_parentOfSubWindows->height());
			m_subWindows.at(i)->show();
		}
		else
		{
			m_subWindows.at(i)->hide();
		}
	}
}

void div1::parentWindowResize( QResizeEvent *ev )
{
	m_subWindows.at(m_nCurrentPage)->resize(ev->size());
}

void div1::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
    Q_UNUSED(subWindow);
    Q_UNUSED(ev);
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
	return QString("div1_1");
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
