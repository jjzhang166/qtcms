#include "previewactivity.h"
#include <guid.h>
#include <QtWebKit/QWebPage>

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
	reinterpret_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall previewactivity::AddRef()
{

	return 0;
}

unsigned long __stdcall previewactivity::Release()
{

	return 0;
}

void previewactivity::Active( QWebFrame * frame)
{
	m_MainFrame = frame;
	m_MainView = (m_MainFrame->page())->view();


	m_MainFrame->addToJavaScriptWindowObject(QString("qob"),this);

	connect(m_MainFrame,SIGNAL(javaScriptWindowObjectCleared),this,SLOT(OnJavaScriptWindowObjectCleared));
	m_MainFrame->evaluateJavaScript(QString("connectEvent('top_act','dblclick',function a(){qob.OnTopActDbClick();});"));
}

void previewactivity::OnJavaScriptWindowObjectCleared()
{
	m_MainFrame->addToJavaScriptWindowObject(QString("qob"),this);
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
