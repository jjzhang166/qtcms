#include "previewactivity.h"
#include <guid.h>
#include <QtWebKit/QWebPage>

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
	qDebug("Addref:%d",m_nRef);
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
		qDebug("delete this;");
		delete this;
	}
	return nRet;
}

void previewactivity::Active( QWebFrame * frame)
{
	m_MainView = (frame->page())->view();
	QWFW_MSGMAP_BEGIN(frame);
	QWFW_MSGMAP("top_act","dblclick","OnTopActDbClick()");
	QWFW_MSGMAP("top_act","mousedown","OnTopActMouseDown()");
	QWFW_MSGMAP("top_act","mouseup","OnTopActMouseUp()");
	QWFW_MSGMAP("top_act","mousemove","OnTopActMouseMove(event.clientX,event.clientY)");
	QWFW_MSGMAP_END;
}

void previewactivity::OnJavaScriptWindowObjectCleared()
{
	QWFW_MSGRESET;
}

void previewactivity::OnTopActDbClick()
{
	if (m_MainView->isMaximized())
	{
		m_MainView->showNormal();
	}
	else
	{
		m_MainView->showMaximized();
	}
}

void previewactivity::OnTopActMouseDown()
{
	qDebug("mouse down");
}

void previewactivity::OnTopActMouseUp()
{
	qDebug("mouse up");
}

void previewactivity::OnTopActMouseMove( int x,int y )
{
	qDebug("mouse move %d,%d",x,y);
}