#include "div8_1.h"
#include <guid.h>

div8_1::div8_1() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),

m_PageSubCount(PAGE_SUBCOUNT),
m_row(4),
m_column(4),
m_singeldisplay(false)
//m_currSubWindows(NULL)
{

}

div8_1::~div8_1()
{

}

void div8_1::setSubWindows( QList<QWidget *> windows,int count )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
}

void div8_1::setParentWindow( QWidget * parent )
{
	m_parentSize = parent->size();
	m_parentOfSubWindows = parent;
}

void div8_1::flush()
{
	reSizeSubWindows();
	int i;
	for (i = 0; i < m_nSubWindowCount; i ++)
	{
		if (i >= m_nCurrentPage*m_PageSubCount && i< (m_nCurrentPage+1)*m_PageSubCount)
		{
			m_subWindows.at(i)->show();
		}
		else
		{
			m_subWindows.at(i)->hide();
		}
	}
}

void div8_1::parentWindowResize( QResizeEvent *ev )
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

	if (m_singeldisplay)
	{
		setSingelDiaplay(NULL);
	}
	else
	{
		setTotalDisplay();
	}
}

void div8_1::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
    Q_UNUSED(ev);
	//qDebug("hi ha %s",subWindow->windowTitle().toAscii().data());

	if (!m_singeldisplay)
	{
		m_nCurrentPage = getSubVindowIndex(subWindow);
		setSingelDiaplay(subWindow);
	}
	else
	{
		
		setTotalDisplay();
	}
	
	return;
}

int div8_1::getSubVindowIndex(QWidget * pSubWindow)
{
	int index ;
	for (index= 0;index<m_nSubWindowCount;index++)
		if(m_subWindows.at(index) == pSubWindow)return index;

	return -1;
}
//void div8_1::adjustSubWindow(int index)
//{
//	int subWidth = m_parentOfSubWindows->size().width()/m_column;
//	int subHeight = m_parentOfSubWindows->size().height()/m_row;
//	int ii = index%m_PageSubCount;
//
//	m_subWindows[index].move(subWidth*(ii%m_column),subHeight*(ii/m_row));
//	m_subWindows[index].resize(subWidth-1,subHeight-1);
//}
//void div8_1::adjustSubWindow(QWidget * pSubWindows)
//{
//	int index = getSubVindowIndex(pSubWindows);
//	if(index!=-1)
//		adjustSubWindow(index);
//}
void div8_1::setSingelDiaplay(QWidget * pSubWindows)
{

	m_PageSubCount = 1;
	//m_currSubWindows = pSubWindows;
	m_singeldisplay = true;
// 	m_nCurrentPage = getSubVindowIndex(pSubWindows);

	flush();
}
void div8_1::setTotalDisplay()
{
	//adjustSubWindow(m_currSubWindows);
	m_PageSubCount = PAGE_SUBCOUNT;
	//m_currSubWindows = NULL;
	m_singeldisplay = false;
	m_nCurrentPage = m_nCurrentPage/m_PageSubCount;

	flush();
}
void div8_1::reSizeSubWindows()
{
	//----------------------------------
	int pox_x=0,pox_y=0;
	bool flagmap[16] ;
	memset(flagmap,0,sizeof(bool)*16);
	for (int i = pox_x;i<pox_x + m_column -1; i++)
		for (int j = pox_y;j<pox_y + m_row -1; j++)
			flagmap[j*m_column+i] = true;
	//
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
		for (int i = indexfirst,j=0; i < indexfirst+m_PageSubCount; i ++)
		{

			int ii = i-indexfirst;
			if(ii == 0)
			{
				m_subWindows.at(i)->move(pox_x*subWidth,pox_y*subHeight);
				m_subWindows.at(i)->resize(subWidth*(m_column-1)-1,subHeight*(m_row-1)-1);
			}
			else
			{
				while(flagmap[j])j++;
				m_subWindows.at(i)->move(subWidth*(j%m_column),subHeight*(j/m_row));
				m_subWindows.at(i)->resize(subWidth-1,subHeight-1);
				j++;
			}
			
		}
	}
	
}

void div8_1::nextPage()
{
	m_nCurrentPage ++;
	if (m_nCurrentPage >= getPages())
	{
		m_nCurrentPage = 0;
	}

	flush();
}

void div8_1::prePage()
{
	m_nCurrentPage --;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = getPages() - 1;
	}

	flush();
}

int div8_1::getCurrentPage()
{
	return m_nCurrentPage;
}

int div8_1::getPages()
{
	return m_nSubWindowCount/m_PageSubCount;
}

QString div8_1::getModeName()
{
	return QString("div8_1");
}

long __stdcall div8_1::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div8_1::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div8_1::Release()
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
