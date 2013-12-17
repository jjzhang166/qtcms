#include "SDLRender.h"
#include <guid.h>

SDLRender::SDLRender() :
m_nRef(0),
pWidget(NULL),
m_pWindow(NULL),
m_pRender(NULL),
m_pTexture(NULL)
{
	
}

SDLRender::~SDLRender()
{

}

int SDLRender::init(int nWidth,int nHeight)
{
	if (pWidget)
	{
		SDL_Init(SDL_INIT_VIDEO);
		if(!m_pWindow)
		{
			qDebug("------------------>1:%d",pWidget->winId());
			m_pWindow = SDL_CreateWindowFrom((void *)pWidget->winId());
		}
		
		qDebug("%s",SDL_GetError());
		if (!m_pWindow)
			return false;
		if(!m_pRender){
			m_pRender = SDL_CreateRenderer( m_pWindow, 2, SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);
		}

		qDebug("%s",SDL_GetError());

		if (m_pTexture)
		{
			SDL_DestroyTexture( m_pTexture );
		}
		 m_pTexture = SDL_CreateTexture( m_pRender,SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, nWidth, nHeight);

		return true;
	}

	return false;
}
int SDLRender::deinit()
{
	if ( m_pTexture != NULL )
	{
		SDL_DestroyTexture( m_pTexture );
		m_pTexture = NULL;
	}

	if ( m_pRender != NULL )
	{
		SDL_DestroyRenderer( m_pRender );
		m_pRender = NULL;
	}

	if (m_pWindow !=NULL)
	{
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = NULL;
	}
	return 0;
}
int SDLRender::setRenderWnd(QWidget * wnd)
{
	if (wnd)
	{
		pWidget = wnd;
		pWidget->winId();
		//qDebug("------------------>2:%p",pWidget->winId());
		return true;
	}
	return false;
}
int SDLRender::render(char *data,char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride,const QString & pixelFormat,int flags)
{
	//qDebug("Width_dec:%d  Height_dec:%d\n",nWidth,nHeight);
	//qDebug("%d\n%d\n%d\n",pYData,pUData,pVData);

	SDL_Rect s_rect,d_rect;
	s_rect.x = 0;
	s_rect.y = 0;
	s_rect.w = nWidth;
	s_rect.h = nHeight;

	int iWidth = 0;
	int iHeight = 0;


	SDL_GetWindowSize( m_pWindow, &iWidth, &iHeight );
	qDebug()<<iWidth;
	qDebug()<<iHeight;
	d_rect.x = 0;
	d_rect.y = 0;
	d_rect.w = iWidth;
	d_rect.h = iHeight;

	int iPitch = nWidth*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_IYUV);

	SDL_UpdateTexture( m_pTexture, &s_rect, data, iPitch );
	SDL_RenderClear( m_pRender );
	//qDebug()<<"m_pTexture";
	//qDebug()<<m_pTexture;
	//qDebug()<<"m_pRender";
	//qDebug()<<m_pRender;
	SDL_RenderCopy( m_pRender, m_pTexture, &s_rect, &d_rect );
	SDL_RenderPresent( m_pRender );


	//qDebug()<<"m_pTexture";
	//qDebug()<<m_pTexture;
	//qDebug()<<"m_pRender";
	//qDebug()<<m_pRender;

	return true;
}
int SDLRender::enable(bool bEnable)
{
	return true;
}
int SDLRender::enableStretch(bool bEnable)
{
	return true;
}
bool SDLRender::isRenderEnable()
{
	return true;
}
bool SDLRender::isStretchEnable()
{
	return true;
}
bool SDLRender::isPixelFormatAvalible(const QString &sFormat)
{
	return true;
}

QString SDLRender::getModeName()
{
	return QString("VideoRender");
}

long __stdcall SDLRender::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IVideoRender == iid)
	{
		*ppv = static_cast<IVideoRender*>(this);
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

unsigned long __stdcall SDLRender::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall SDLRender::Release()
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