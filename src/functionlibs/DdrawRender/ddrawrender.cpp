#include "ddrawrender.h"
#include <guid.h>
#include <QtCore/QLibrary>
#include <QtCore/QCoreApplication>

typedef IDDrawRender * (*lpCreateObject)();
typedef void (*lpReleaseObj)(IDDrawRender *);

DdrawRender::DdrawRender():
m_nRef(0)
,m_bStretch(false)
,m_bEnable(true)
{
	QLibrary Module("DDrawRenderObject.dll");
	lpCreateObject pFun = (lpCreateObject)Module.resolve("CreateObject");
	m_renderObj = pFun();
}

DdrawRender::~DdrawRender()
{
	if (NULL != m_renderObj)
	{
		QLibrary Module("DDrawRenderObject.dll");
		lpReleaseObj pFun = (lpReleaseObj)Module.resolve("ReleaseObject");
		pFun(m_renderObj);
	}
}

int DdrawRender::init( int nWidth,int nHeight )
{
	return m_renderObj->init(nWidth,nHeight);
}

int DdrawRender::deinit()
{
	return m_renderObj->deinit();
}

int DdrawRender::setRenderWnd( QWidget * wnd )
{
	return m_renderObj->setRenderWnd(wnd->winId());
}

int DdrawRender::render( char *data,char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride,const QString & pixelFormat,int flags )
{
	return m_renderObj->render(pYData,pUData,pVData,nWidth,nHeight,nYStride,nUVStride,lineStride);
}

int DdrawRender::enable( bool bEnable )
{
	m_bEnable = bEnable;
	m_renderObj->enable(bEnable);
	return 0;
}

int DdrawRender::enableStretch( bool bEnable )
{
	m_bStretch = bEnable;
	m_renderObj->enableStretch(bEnable);
	return 0;
}

bool DdrawRender::isRenderEnable()
{
	return m_bEnable;
}

bool DdrawRender::isStretchEnable()
{
	return m_bStretch;
}

bool DdrawRender::isPixelFormatAvalible( const QString &sFormat )
{
	return QString("YV12") == sFormat;
}

long __stdcall DdrawRender::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IVideoRender == iid)
	{
		*ppv = static_cast<IVideoRender*>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IVideoRenderDigitalZoom==iid)
	{
		*ppv = static_cast<IVideoRenderDigitalZoom*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall DdrawRender::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall DdrawRender::Release()
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

bool DdrawRender::addExtendWnd( QWidget * wnd,const QString sName )
{
	char *cName;
	QByteArray ba=sName.toLatin1();
	cName=ba.data();
	return m_renderObj->addExtendWnd(wnd->winId(),cName);
}

void DdrawRender::setRenderRect( int nX,int nY,int nWidth,int nHeight )
{
	return m_renderObj->setRenderRect(nX,nY,nWidth,nHeight);
}

void DdrawRender::removeExtendWnd( const QString sName )
{
	char *cName;
	QByteArray ba=sName.toLatin1();
	cName=ba.data();
	return m_renderObj->removeExtendWnd(cName);
}

void DdrawRender::setRenderRectPen( int nLineWidth,int nR,int nG,int nB )
{
	return m_renderObj->setRenderRectPen(nLineWidth,nR,nG,nB);
}
