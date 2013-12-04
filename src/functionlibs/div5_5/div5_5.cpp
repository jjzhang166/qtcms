#include "div5_5.h"
#include <qwfw.h>

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <guid.h>

div5_5::div5_5() :
m_nRef(0),
m_nSubWindowCount(0),
m_parentOfSubWindows(NULL),
m_nCurrentPage(0),
m_nTotalWindowsCount(0),
m_nRow(5),
m_nColumn(5),
m_bIsMax(false)
{

}

div5_5::~div5_5()
{

}

void div5_5::setSubWindows( QList<QWidget *> windows,int count )
{
	m_subWindows = windows;
	m_nSubWindowCount = count;
	if (count %(m_nRow * m_nColumn) != 0)
	    m_nTotalWindowsCount = count/(m_nRow * m_nColumn) + 1;
	else
		m_nTotalWindowsCount = count/(m_nRow * m_nColumn) ;
}

void div5_5::setParentWindow( QWidget * parent )
{
	m_parentOfSubWindows = parent;
}


void div5_5::flush()
{
	int nWidth     = 0;
	int nHeight    = 0;
	int nX          = 0;
	int nY          = 0;
	int nPositionX = 0;
	int nPositionY = 0;
	if(!m_bIsMax)  //当没有子窗口被放大时
	{
		nWidth  = (m_parentOfSubWindows->width()) / m_nRow;
		nHeight = (m_parentOfSubWindows->height()) / m_nColumn;
		int i ; //屏幕最上角的子窗口下标
		if (m_nSubWindowCount % (m_nRow * m_nColumn) == 0)
		{
			i = m_nRow * m_nColumn * m_nCurrentPage;
		} 
		else
		{
			if (m_nCurrentPage  ==  m_nSubWindowCount / (m_nRow * m_nColumn))// 为最后一页时
			{
				i = m_nSubWindowCount - m_nRow * m_nColumn;  // 仍填充一页
			}
			else
				i = m_nRow * m_nColumn * m_nCurrentPage;
		}


		for(int j = 0;j < m_nSubWindowCount;j++)       // m_nSubWindowCount:总的子窗口数目
		{
			if (j >= i && j < i + m_nRow * m_nColumn)
			{
				nPositionX = nX * nWidth;
				nPositionY = nY * nHeight;
				++nX;
				if (nX == m_nRow)
				{
					nX %= m_nRow;
					++nY;
					nY = ((nY >= m_nColumn) ? nY%m_nColumn : nY);
				}
				m_subWindows.at(j)->setGeometry(nPositionX,nPositionY,nWidth,nHeight);			
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
void div5_5::parentWindowResize( QResizeEvent *ev )
{
	if (m_nTotalWindowsCount == m_nSubWindowCount)
	{
		m_subWindows.at(m_nCurrentPage)->resize(ev->size());
		m_subWindows.at(m_nCurrentPage)->show();
		
		for (int i = 0; i < m_nSubWindowCount ; i++)
		{
			if (i != m_nCurrentPage)
			{
				m_subWindows.at(m_nCurrentPage)->hide();
			} 
			else
				continue;
		}
	}
	else
	{
		int nPositionX = 0 ;
		int nPositionY = 0 ;
		int nX = 0;      // 一行的第x个子窗口
		int nY = 0;      // 第y列
		int nWidth     = ev->size().width()/ m_nRow;
		int nHeight    = ev->size().height()/ m_nColumn;

		int i ; //屏幕最上角的子窗口下标
		if (m_nSubWindowCount % (m_nRow * m_nColumn) == 0)
		{
			i = m_nRow * m_nColumn * m_nCurrentPage;
		} 
		else
		{
			if (m_nCurrentPage  ==  m_nSubWindowCount / (m_nRow * m_nColumn))// 为最后一页时
			{
				i = m_nSubWindowCount - m_nRow * m_nColumn;  // 仍填充满一页
			}
			else
				i = m_nRow * m_nColumn * m_nCurrentPage;
		}

		for(int j = 0;j < m_nSubWindowCount;j++)
		{
			if (j >= i && j < i + m_nRow * m_nColumn )
			{
				nPositionX = nX * nWidth;
				nPositionY = nY * nHeight;
				++nX;
				if (nX == m_nRow)
				{
					nX %= m_nRow;
					++nY;
					nY = ((nY >= m_nColumn) ? nY%m_nColumn : nY);
				}
				m_subWindows.at(j)->setGeometry(nPositionX,nPositionY,nWidth,nHeight);			
				m_subWindows.at(j)->show();
			}
			else
				m_subWindows.at(j)->hide();
		}  // End of  for(int j = 0;j < m_nSubWindowCount;j++)
	}
}


void div5_5::subWindowDblClick( QWidget *subWindow,QMouseEvent * ev )
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
	    ||subWindow->frameGeometry().height() == m_parentOfSubWindows->height()
	  )
	{
		m_bIsMax = false;
		m_nCurrentPage = j/(m_nRow * m_nColumn);

		if (m_nSubWindowCount %(m_nRow * m_nColumn) != 0)
			m_nTotalWindowsCount = m_nSubWindowCount/(m_nRow * m_nColumn) + 1;
		else
			m_nTotalWindowsCount = m_nSubWindowCount/(m_nRow * m_nColumn) ;
		
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
		subWindow->setGeometry( 0,
			                     0,
			                    m_parentOfSubWindows->width(),
			                    m_parentOfSubWindows->height()
								);
		subWindow->show();
	}
	return;
}

void div5_5::nextPage()
{
	m_nCurrentPage ++;
	if (m_nCurrentPage >= m_nTotalWindowsCount)
	{
		m_nCurrentPage = 0;
	}
	flush();
	
}

void div5_5::prePage()
{
	m_nCurrentPage --;
	if (m_nCurrentPage < 0)
	{
		m_nCurrentPage = m_nTotalWindowsCount - 1;
	}

	flush();
}

int div5_5::getCurrentPage()
{
	return m_nCurrentPage;
}

int div5_5::getPages()
{
	return m_nTotalWindowsCount;
}

QString div5_5::getModeName()
{
	return QString("div5_5");
}

long __stdcall div5_5::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall div5_5::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall div5_5::Release()
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