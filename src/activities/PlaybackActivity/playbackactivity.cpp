#include "playbackactivity.h"
#include <guid.h>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebElementCollection>
#include <QtCore/QSize>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QDebug>
PlaybackActivity::PlaybackActivity():
m_nRef(0)
{

}

PlaybackActivity::~PlaybackActivity()
{

}

long __stdcall PlaybackActivity::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall PlaybackActivity::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	qDebug("Addref:%d",m_nRef);
	return m_nRef;
}

unsigned long __stdcall PlaybackActivity::Release()
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

void PlaybackActivity::Active( QWebFrame * frame )
{
	m_MainView = (frame->page())->view();
	m_frame=frame;
	QWFW_MSGMAP_BEGIN(frame);
	/*QWFW_MSGMAP("app_top","dblclick","OnTopActDbClick()");*/
	QWFW_MSGMAP("app_close_window","click","OnCloseWindow()");
	/*QWFW_MSGMAP("app_maxsize","click","OnMaxsizeWindow()");*/
	QWFW_MSGMAP("app_minsize","click","OnMinsizeWindow()");
	QWFW_MSGMAP_END;
}

void PlaybackActivity::OnTopActDbClick()
{
	if (m_MainView->isMaximized()||m_MainView->size().height()==QApplication::desktop()->screenGeometry().height()||m_MainView->size().width()==QApplication::desktop()->screenGeometry().width())
	{
		m_MainView->showNormal();
		QRect rcScreen = QApplication::desktop()->screenGeometry();

		QSize currentSize=m_MainView->size();
		if (rcScreen.height()*2/3>600)
		{
			currentSize.setHeight(rcScreen.height()*2/3);
		}else{
			currentSize.setHeight(600);
		}
		if (rcScreen.width()*2/3>1000)
		{
			currentSize.setWidth(rcScreen.width()*2/3);
		}
		else{
			currentSize.setWidth(1000);
		}
		m_MainView->resize(currentSize);
		qDebug()<<m_MainView;
		int nX=rcScreen.width()-currentSize.width();
		int nY=rcScreen.height()-currentSize.height();
		m_MainView->move(nX/2,nY/2);
	}
	else
	{
		/*m_MainView->showMaximized();*/
		m_MainView->showFullScreen();
	}
}

void PlaybackActivity::OnCloseWindow()
{
	m_MainView->close();
}

void PlaybackActivity::OnMaxsizeWindow()
{
	if (m_MainView->isMaximized()||m_MainView->size().height()==QApplication::desktop()->screenGeometry().height()||m_MainView->size().width()==QApplication::desktop()->screenGeometry().width())
	{
		m_MainView->showNormal();
		QRect rcScreen = QApplication::desktop()->screenGeometry();

		QSize currentSize=m_MainView->size();
		currentSize.setHeight(rcScreen.height()*2/3);
		currentSize.setWidth(rcScreen.width()*2/3);
		m_MainView->resize(currentSize);
		int nX=rcScreen.width()-currentSize.width();
		int nY=rcScreen.height()-currentSize.height();
		m_MainView->move(nX/2,nY/2);
	}else
	{
		m_MainView->showMaximized();
	}
}

void PlaybackActivity::OnMinsizeWindow()
{
	m_MainView->showMinimized();
}
