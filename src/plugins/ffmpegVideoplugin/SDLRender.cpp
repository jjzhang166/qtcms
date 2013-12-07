#include "SDLRender.h"	
#include <WTypes.h>
#include <QtGui/QMessageBox>
//#include <SDL_Render.h>
CSDLRender::CSDLRender(void)
{
	pWindow = nullptr;
}


CSDLRender::~CSDLRender(void)
{
	if (pWindow != nullptr)
	{
		SDL_DestroyWindow(pWindow);
	}
}

IVideoRender::ErrorCode CSDLRender::Init(int nWidth,int nHeight)
{

	return SUCCESS;
}

IVideoRender::ErrorCode CSDLRender::DeInit()
{

	return SUCCESS;
}

IVideoRender::ErrorCode CSDLRender::SetRenderWnd(WId qWid)
{
	m_csWnd.lock();
	SDL_Init(SDL_INIT_VIDEO);
	 pWindow = SDL_CreateWindowFrom((void *)qWid);
	qDebug("%s",SDL_GetError());
	m_csWnd.unlock();
	return SUCCESS;
}

IVideoRender::ErrorCode CSDLRender::Render(char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride)
{
// 	qDebug("SDLRender!!");
// 	char variable[64];
// 	sprintf(variable, "SDL_WINDOWID=0x%lx", m_hPlayWnd);
// 	SDL_putenv(variable);
// //	putenv("SDL_VIDEO_WINDOW_POS=0,0");
// 	RECT wrect;
// 	GetWindowRect((HWND)m_hPlayWnd,&wrect);
// 	int w = wrect.right - wrect.left;
// 	int h = wrect.bottom - wrect.top;
// 	if (w % 2 != 0)
// 	{
// 		w -= 1;
// 	}
// 	if (h % 2 != 0)
// 	{
// 		h -= 1;
// 	}
// 	qDebug("%d:%d",w,h);
// 	
// 	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
// 	{
// 		qDebug("Could not initialize SDL!!");
// 		return E_NOT_INIT;
// 	}
// 
// 	ps_screen = SDL_SetVideoMode(w,h,0,SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_NOFRAME | SDL_RESIZABLE);
// 
// 	if (!ps_screen)
// 	{
// 		qDebug("SDL:could not set video mode!");
// 		return E_FAILED;
// 	}
// 	/*SDL_WM_SetCaption( "Test Window", NULL);*/
// 
// 	ps_bmp = SDL_CreateYUVOverlay(nWidth,nHeight, SDL_YV12_OVERLAY,ps_screen);
// 
// 	ps_bmp->pixels[0] = (unsigned _int8*)pYData;
// 	ps_bmp->pixels[2] = (unsigned _int8*)pUData;
// 	ps_bmp->pixels[1] = (unsigned _int8*)pVData;
// 
// 	ps_bmp->pitches[0] = nWidth;
// 	ps_bmp->pitches[1] = nHeight;
// 	ps_bmp->pitches[2] = nYStride;
// 	ps_bmp->pitches[3] = nUVStride;
// 
// 	s_rect.x = 0;
// 	s_rect.y = 0;
// 	s_rect.h = h;
// 	s_rect.w = w;
// 
//  	SDL_LockYUVOverlay(ps_bmp);
// 	SDL_DisplayYUVOverlay(ps_bmp, &s_rect);
// //	SDL_UpdateRects(ps_screen,1,&s_rect);
// 	SDL_UnlockYUVOverlay(ps_bmp);
// 	return SUCCESS;
	
	char *pixel;
	int nSize = nWidth * nHeight;
	pixel = new char[nSize * 3 / 2];
	memcpy(pixel,pYData,nSize);
	memcpy(pixel + nSize ,pVData, nSize / 4);
	memcpy(pixel + nSize * 5 /4,pUData,nSize / 4);


	int iWidth = 0;
	int iHeight = 0;
	SDL_GetWindowSize( pWindow, &iWidth, &iHeight );
	int iPitch = nWidth*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_YV12);    
	s_rect.x = 0;
	s_rect.y = 0;
	s_rect.w = nWidth;
	s_rect.h = nHeight;

	d_rect.x = 0;
	d_rect.y = 0;
	d_rect.w = iWidth;
	d_rect.h = iHeight;

	//创建渲染器，第二个参数为选用的画图驱动，0代表d3d,1代表opengl，2代表sofeware
	SDL_Renderer * pRender = SDL_CreateRenderer( pWindow, 2 , SDL_RENDERER_ACCELERATED );

	SDL_Texture * pTexture = SDL_CreateTexture( pRender,SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, nWidth, nHeight);

	SDL_UpdateTexture( pTexture, &s_rect, pixel, iPitch );
	SDL_RenderClear( pRender );
	SDL_RenderCopy( pRender, pTexture, &s_rect, &s_rect );
	SDL_RenderPresent( pRender );

	if ( pTexture != NULL )
	{
		SDL_DestroyTexture( pTexture );
		pTexture = NULL    ;
	}

	if ( pRender != NULL )
	{
		SDL_DestroyRenderer( pRender );
		pRender = NULL;
	}

	if (pixel != nullptr)
	{
		delete pixel;
		pixel = nullptr;
	}
	return SUCCESS;
}

bool CSDLRender::Enable(bool bEnable)
{

	return bEnable;
}

void CSDLRender::SetRecStatus(bool bRec)
{

}

int CSDLRender::InitGlobalResource(WId qWid)
{

	return 0;
}

int CSDLRender::DeInitGlobalResource()
{

	return 0;
}