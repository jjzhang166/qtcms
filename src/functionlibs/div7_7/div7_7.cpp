#include "div7_7.h"
#include <guid.h>

div7_7::div7_7() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),

m_PageSubCount(PAGE_SUBCOUNT),
m_row(7),
m_column(7),
m_singeldisplay(false)
//m_currSubWindows(NULL)
{

}

div7_7::~div7_7()
{

}

void div7_7::setSubWindows( QList<QWidget *> windows,int count  )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
}

void div7_7::setParentWindow( QWidget * parent )
{
	m_parentSize = parent->size();
	m_parentOfSubWindows = parent;
}

void div7_7::flush()
{
	reSizeSubWindows();
	int i;
	for (i = 0; i < m_nSubWindowCount || i< (m_nCurrentPage+1)*m_PageSubCount; i ++)
	{
		if (i >= m_nCurrentPage*m_PageSubCount && i< (m_nCurrentPage+1)*m_PageSubCount)
		{
			m_subWindows.at(i%m_nSubWindowCount)->show();
		}
		else
		{
			m_subWindows.at(i%m_nSubWindowCount)->hide();
		}
	}
}

void div7_7::parentWindowResize( QResizeEvent *ev )
{
	m_parentSize = ev->size();
	/*int subWidth = .width()/m_column;
	int subHeight = ev->size().height()/m_row;
	for (int i = 0; i < m_nSubWindowCount; i ++)
	{

		int ii = i%m_PageSubCount;
		m_subWindows[i].move(subWidth*(ii%m_column),subHeight*(ii/m_row));
		m_subWindows[i].resize(subWidth-1,subHeight-1);
	}*/
	setTotalDisplay();
}

void div7_7::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
	qDebug("hi ha %s",subWindow->windowTitle().toAscii().data());

	if (!m_singeldisplay)
	{
		setSingelDiaplay(subWindow);
	}
	else
	{
		
		setTotalDisplay();
	}
	
	return;
}

int div7_7::getSubVindowIndex(QWidget * pSubWindow)
{
	int index ;
	for (index= 0;index<m_nSubWindowCount;index++)
		if(m_subWindows.at(index) == pSubWindow)return index;

	return -1;
}
//void div7_7::adjustSubWindow(int index)
//{
//	int subWidth = m_parentOfSubWindows->size().width()/m_column;
//	int subHeight = m_parentOfSubWindows->size().height()/m_row;
//	int ii = index%m_PageSubCount;
//
//	m_subWindows[index].move(subWidth*(ii%m_column),subHeight*(ii/m_row));
//	m_subWindows[index].resize(subWidth-1,subHeight-1);
//}
//void div7_7::adjustSubWindow(QWidget * pSubWindows)
//{
//	int index = getSubVindowIndex(pSubWindows);
//	if(index!=-1)
//		adjustSubWindow(index);
//}
void div7_7::setSingelDiaplay(QWidget * pSubWindows)
{

	m_PageSubCount = 1;
	//m_currSubWindows = pSubWindows;
	m_singeldisplay = true;
	m_nCurrentPage = getSubVindowIndex(pSubWindows);

	flush();
}
void div7_7::setTotalDisplay()
{
	//adjustSubWindow(m_currSubWindows);
	m_PageSubCount = PAGE_SUBCOUNT;
	//m_currSubWindows = NULL;
	m_singeldisplay = false;
	m_nCurrentPage = m_nCurrentPage/m_PageSubCount;

	flush();
}
void div7_7::reSizeSubWindows()
{
	int subWidth ;
	int subHeight ;
	if (m_singeldisplay)
	{
		subWidth=m_parentSize.width();
		subHeight = m_parentSize.height();
		m_subWindows.at(m_nCurrentPage)->move(0,0);
		m_subWindows.at(m_nCurrentPage)->resize(subWidth-1,subHeight-1);
	}
	else
	{
		int subWidth = m_parentSize.width()/m_column;
		int subHeight = m_parentSize.height()/m_row;
		int indexfirst = m_PageSubCount*m_nCurrentPage;
		for (int i = indexfirst; i < indexfirst+m_PageSubCount; i ++)
		{

			int ii = i-indexfirst;
			m_subWindows.at(i%m_nSubWindowCount)->move(subWidth*(ii%m_column),subHeight*(ii/m_row));
			m_subWindows.at(i%m_nSubWindowCount)->resize(subWidth-1,subHeight-1);
		}
	}
	
}

void div7_7::nextPage()
{
	m_nCurrentPage ++;
	if (m_nCurrentPage >= getPages())
	{
		m_nCurrentPage = 0;
	}

	flush();
}

void div7_7::prePage()
{
	m_nCurrentPage --;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = getPages() - 1;
	}

	flush();
}

int div7_7::getCurrentPage()
{
	return m_nCurrentPage;
}

int div7_7::getPages()
{
	int pages = m_nSubWindowCount/m_PageSubCount;
	pages += (m_nSubWindowCount%m_PageSubCount)==0?0:1;
	return pages;
}

QString div7_7::getModeName()
{
	return QString("div7_7");
}

long __stdcall div7_7::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div7_7::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div7_7::Release()
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