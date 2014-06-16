#include "previewactivity.h"
#include <qdebug.h>
#include <guid.h>
//#include <QtWebKit/QtWebKit>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebElementCollection>
#include <QtCore/QSize>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>

previewactivity::previewactivity():
m_nRef(0)
{

}

long __stdcall previewactivity::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IActivities == iid)
	{
		*ppv = static_cast<IActivities *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall previewactivity::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall previewactivity::Release()
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

void previewactivity::Active( QWebFrame * frame)
{
	m_MainView = (frame->page())->view();
	m_frame=frame;
	QWFW_MSGMAP_BEGIN(frame);
	QWFW_MSGMAP("app_top","dblclick","OnTopActDbClick()");
	QWFW_MSGMAP("app_close_window","click","OnCloseWindow()");
	QWFW_MSGMAP("app_maxsize","click","OnMaxsizeWindow()");
	QWFW_MSGMAP("app_minsize","click","OnMinsizeWindow()");
	QWFW_MSGMAP_END;
}

void previewactivity::OnTopActDbClick()
{
	if (m_MainView->isMaximized())
	{
		// set to normal size
		m_MainView->showNormal();

		// get screen size
		QRect rcScreen = QApplication::desktop()->screenGeometry();
		// get current size
		QSize currentSize=m_MainView->size();

		if (currentSize.width() < 1072)
		{
			currentSize.setWidth(1072);
		}
		if (currentSize.height() < 714)
		{
			currentSize.setHeight(714);
		}
		m_MainView->resize(currentSize);

		// reset x coordinate
		int nX = rcScreen.width() - currentSize.width();
		nX = nX > 0 ? nX : 0;
		nX /= 2;

		// reset y coordinate
		int nY = rcScreen.height() - currentSize.height();
		nY = nY > 0 ? nY : 0;
		nY /= 2;

		// apply the 
		m_MainView->move(nX,nY);
	}
	else
	{
		m_MainView->showMaximized();
	}
}

void previewactivity::OnCloseWindow()
{
	m_MainView->close();
}

void previewactivity::OnMaxsizeWindow()
{
	if (!m_MainView->isMaximized())
	{
		m_MainView->showMaximized();
	}
}

void previewactivity::OnMinsizeWindow()
{
	if (!m_MainView->isMinimized())
	{
		m_MainView->showMinimized();
	}
}

