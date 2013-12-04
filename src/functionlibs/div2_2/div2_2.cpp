#include "div2_2.h"
#include <qwfw.h>

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <guid.h>

div2_2::div2_2() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),
m_nTotalWindowsCount(0),
m_nRow(2),
m_nColumn(2),
m_bIsMax(false)
{

}

div2_2::~div2_2()
{

}

void div2_2::setSubWindows(QList<QWidget *> windows,int count  )
{
	m_subWindows = windows;
	m_nTotalWindowsCount = count/(m_nRow * m_nColumn);
	m_nSubWindowCount = count;
}

void div2_2::setParentWindow( QWidget * parent )
{
	m_parentOfSubWindows = parent;
}


void div2_2::flush()
{
	int nWidth  = 0;
	int nHeight = 0;
 	
	if(!m_bIsMax)  //当没有子窗口被放大时
	{
		nWidth  = (m_parentOfSubWindows->width()) / m_nRow;
		nHeight = (m_parentOfSubWindows->height()) / m_nColumn;
		int i = m_nRow * m_nColumn * m_nCurrentPage;// 当前页第一个子窗口

		for(int j = 0;j < m_nSubWindowCount;j++)       //  总的子窗口数目
		{
			if (j >= i && j < i + m_nRow * m_nColumn)
			{
				switch(j%(m_nRow * m_nColumn ))
				{
				case 0:
					m_subWindows.at(j)->setGeometry(0,0,nWidth,nHeight);
					break;
				case 1:
					m_subWindows.at(j)->setGeometry(nWidth,0,nWidth,nHeight);
					break;
				case 2:
					m_subWindows.at(j)->setGeometry(0 ,nHeight,nWidth,nHeight);
					break;
				case 3:
					m_subWindows.at(j)->setGeometry( nWidth, nHeight,nWidth,nHeight);
					break;
				default:
					break;
				}
				m_subWindows.at(j)->show();
			}
			else
				m_subWindows.at(j)->hide();
		}  // End of  for(int j = 0;j < m_nSubWindowCount;j++) 
	}      // End of  if(!m_bIsMax)
	else   //当有子窗口被放大时
	{
		m_subWindows.at(m_nCurrentPage)->setGeometry(0     ,
			                                     0     ,
			                                     m_parentOfSubWindows->width()  ,
												 m_parentOfSubWindows->height()
												 );
		m_subWindows.at(m_nCurrentPage)->show();
		for (int i = 0; i < m_nSubWindowCount ; i++)
		{
			if (i != m_nCurrentPage)
			{
				m_subWindows.at(i)->hide();
			} 
			else
				continue;
		}
	} // End of else 
}

// 父窗口大小变动   
void div2_2::parentWindowResize( QResizeEvent *ev )
{
	
	if (m_nTotalWindowsCount == m_nSubWindowCount)
	{
		m_subWindows.at(m_nCurrentPage)->resize(ev->size());
		m_subWindows.at(m_nCurrentPage)->show();
		
		for (int i = 0; i < m_nSubWindowCount ; i++)
		{
			if (i != m_nCurrentPage)
			{
				m_subWindows.at(i)->hide();
			} 
			else
				continue;
		}
	}
	else
	{
		int nPositionX =  0 ;
		int nPositionY =  0;
		int nWidth     = ev->size().width()/ m_nRow;
		int nHeight    = ev->size().height()/ m_nColumn;

		int i = m_nRow * m_nColumn * m_nCurrentPage;

		for(int j = 0;j < m_nSubWindowCount;j++)
		{
			if (j >= i && j < i + m_nRow * m_nColumn )
			{
				switch(j%(m_nRow * m_nColumn ))
				{
				case 0:
					nPositionX = 0;
					nPositionY = 0;
					break;
				case 1:
					nPositionX = nWidth;
					nPositionY = 0;
					break;
				case 2:
					nPositionX = 0;
					nPositionY = nHeight;
					break;
				case 3:
					nPositionX = nWidth;
					nPositionY = nHeight;
					break;
				default:
					break;
				}  // End of switch(j%(m_nRow * m_nColumn ))

				m_subWindows.at(j)->setGeometry(nPositionX,nPositionY,nWidth,nHeight);			
				m_subWindows.at(j)->show();
			}
			else
				m_subWindows.at(j)->hide();
		}  // End of  for(int j = 0;j < m_nSubWindowCount;j++)
	}
	
}


void div2_2::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
{
	int j ;
	for ( j = 0; j < m_nSubWindowCount;j++)
	{
		if (m_subWindows.at(j) == subWindow)
		{
			qDebug("%d window is double Clicked.",j);
			break;
		}
	}

	//qDebug("hi ha %s",subWindow->windowTitle().toAscii().data());

	if( subWindow->frameGeometry().width() == m_parentOfSubWindows->width() 
	    ||subWindow->frameGeometry().height() == m_parentOfSubWindows->height())
	{
		m_bIsMax = false;
		m_nCurrentPage = j/(m_nRow * m_nColumn);
		m_nTotalWindowsCount = m_nSubWindowCount / (m_nRow * m_nColumn);
		flush();
	}
	else
    {
		for(int i = 0; i < m_nSubWindowCount; i++)
		{
			
			m_subWindows.at(i)->hide();
		}
		m_bIsMax             = true;
		m_nCurrentPage       = j;
		m_nTotalWindowsCount = m_nSubWindowCount;
		subWindow->setGeometry(0,
			                    0,
			                    m_parentOfSubWindows->width(),
			                    m_parentOfSubWindows->height()
								);
		subWindow->show();
	}
	return;
}

void div2_2::nextPage()
{
	m_nCurrentPage ++;
	if (m_nCurrentPage >= m_nTotalWindowsCount)
	{
		m_nCurrentPage = 0;
	}
	flush();
	
}

void div2_2::prePage()
{
	m_nCurrentPage --;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = m_nTotalWindowsCount - 1;
	}

	flush();
}

int div2_2::getCurrentPage()
{
	return m_nCurrentPage;
}

int div2_2::getPages()
{
	return m_nTotalWindowsCount;
}

QString div2_2::getModeName()
{
	return QString("div2_2");
}

long __stdcall div2_2::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div2_2::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div2_2::Release()
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