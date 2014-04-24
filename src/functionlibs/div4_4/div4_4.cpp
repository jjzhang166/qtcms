#include "div4_4.h"
#include <guid.h>
#include <QStateMachine>
#include <QPropertyAnimation>

div4_4::div4_4() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),

m_PageSubCount(16),
m_row(4),
m_column(4),
m_singeldisplay(false)
{

}

div4_4::~div4_4()
{

}

void div4_4::setSubWindows(QList<QWidget *> windows,int count )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
}

void div4_4::setParentWindow( QWidget * parent )
{
	m_parentSize = parent->size();
	m_parentOfSubWindows = parent;
}

void div4_4::flush()
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

void div4_4::parentWindowResize( QResizeEvent *ev )
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
		setTotalDisplay(NULL);
	}
}

void div4_4::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
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
		
		setTotalDisplay(subWindow);
	}
	
	return;
}

int div4_4::getSubVindowIndex(QWidget * pSubWindow)
{
	int index ;
	for (index= 0;index<m_nSubWindowCount;index++)
		if(m_subWindows.at(index) == pSubWindow)
			return index;

	return -1;
}
//void div4_4::adjustSubWindow(int index)
//{
//	int subWidth = m_parentOfSubWindows->size().width()/m_column;
//	int subHeight = m_parentOfSubWindows->size().height()/m_row;
//	int ii = index%m_PageSubCount;
//
//	m_subWindows[index].move(subWidth*(ii%m_column),subHeight*(ii/m_row));
//	m_subWindows[index].resize(subWidth-1,subHeight-1);
//}
//void div4_4::adjustSubWindow(QWidget * pSubWindows)
//{
//	int index = getSubVindowIndex(pSubWindows);
//	if(index!=-1)
//		adjustSubWindow(index);
//}
void div4_4::setSingelDiaplay(QWidget * pSubWindows)
{

	m_PageSubCount = 1;
	//m_currSubWindows = pSubWindows;
	m_singeldisplay = true;
// 	m_nCurrentPage = getSubVindowIndex(pSubWindows);

	//QState state1 ;
	//state1.assignProperty(pSubWindows,"geometry",QRect(0,0,m_parentSize.width(),m_parentSize.height()));
	//QPropertyAnimation *ani=new QPropertyAnimation(pSubWindows,"geometry");
	//ani->setStartValue(pSubWindows->geometry());
	//ani->setEndValue(QRect(0,0,m_parentSize.width(),m_parentSize.height()));
	//ani->setDuration(300);
	//ani->setEasingCurve(QEasingCurve::OutSine);
	//ani->start(QAbstractAnimation::DeleteWhenStopped);
	
	flush();
}
void div4_4::setTotalDisplay(QWidget * pSubWindow)
{
	//adjustSubWindow(m_currSubWindows);
	m_PageSubCount = PAGE_SUBCOUNT;
	//m_currSubWindows = NULL;
	m_singeldisplay = false;
	m_nCurrentPage = m_nCurrentPage/m_PageSubCount;
// 	if(pSubWindow)
// 	{
// 		int index = getSubVindowIndex(pSubWindow);
// 		if (index!=-1)
// 		{
//			int subWidth = m_parentSize.width()/m_column;
//			int subHeight = m_parentSize.height()/m_row;
//			int indexfirst = m_PageSubCount*m_nCurrentPage;

//			int ii = index-indexfirst;

			//QPropertyAnimation *ani=new QPropertyAnimation(pSubWindow,"geometry");
			//ani->setStartValue(pSubWindow->geometry());
			//ani->setEndValue(QRect(subWidth*(ii%m_column),subHeight*(ii/m_row),subWidth-1,subHeight-1));
			//ani->setDuration(300);
			//ani->setEasingCurve(QEasingCurve::OutSine);
			//ani->start(QAbstractAnimation::DeleteWhenStopped);
// 		}
		
//	}
	

	flush();
}
void div4_4::reSizeSubWindows()
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
			m_subWindows.at(i)->move(subWidth*(ii%m_column),subHeight*(ii/m_row));
			m_subWindows.at(i)->resize(subWidth-1,subHeight-1);
		}
	}
	
}

void div4_4::nextPage()
{
	m_nCurrentPage ++;
	if (m_nCurrentPage >= getPages())
	{
		m_nCurrentPage = 0;
	}

	flush();
}

void div4_4::prePage()
{
	m_nCurrentPage --;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = getPages() - 1;
	}

	flush();
}

int div4_4::getCurrentPage()
{
	return m_nCurrentPage;
}

int div4_4::getPages()
{
	return m_nSubWindowCount/m_PageSubCount;
}

QString div4_4::getModeName()
{
	return QString("div4_4");
}

long __stdcall div4_4::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div4_4::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div4_4::Release()
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
