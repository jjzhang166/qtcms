// DDrawRenderObject.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DDrawRenderObject.h"
#include "DDMutex.h"

CDDMutex				g_csDDraw;
CDDMutex                g_csClipper;
LPDIRECTDRAW7           g_pDirectDraw7 = NULL;
LPDIRECTDRAWSURFACE7    g_pPrimarySurface = NULL;	
LPDIRECTDRAWCLIPPER	    g_pClipper = NULL;
bool                    g_bDDrawInit = false;
int						g_nDDrawRef = 0;

void InitDDrawGlobal()
{
	g_csDDraw.Lock();
	if (g_nDDrawRef > 0)
	{
		return;
	}

	// InitSource
	HRESULT hr;
	hr = DirectDrawCreateEx(NULL,(LPVOID *)&g_pDirectDraw7,IID_IDirectDraw7,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7 = NULL;
		g_csDDraw.Unlock();
		return;
	}

	hr = g_pDirectDraw7->SetCooperativeLevel(NULL,DDSCL_NORMAL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csDDraw.Unlock();
		return ;
	}

	hr = g_pDirectDraw7->CreateClipper(0,&g_pClipper,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csDDraw.Unlock();
		return ;
	}

	DDSURFACEDESC2 ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	hr = g_pDirectDraw7->CreateSurface(&ddsd,&g_pPrimarySurface,NULL);
	if (FAILED(hr))
	{
		g_pClipper->Release();
		g_pClipper = NULL;
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csDDraw.Unlock();
		return ;
	}

	hr = g_pPrimarySurface->SetClipper(g_pClipper);
	if (FAILED(hr))
	{
		g_pPrimarySurface->Release();
		g_pPrimarySurface = NULL;
		g_pClipper->Release();
		g_pClipper = NULL;
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csDDraw.Unlock();
		return ;
	}

	// ref
	g_nDDrawRef ++;
	g_csDDraw.Unlock();
}

void DeinitDDrawGlobal()
{
	g_csDDraw.Lock();
	if (g_nDDrawRef > 0)
	{
		g_nDDrawRef --;
		if (0 == g_nDDrawRef)
		{
			// release resource
			if (NULL != g_pPrimarySurface)
			{
				g_pPrimarySurface->Release();
				g_pPrimarySurface = NULL;
			}

			if (NULL != g_pClipper)
			{
				g_pClipper->Release();
				g_pClipper = NULL;
			}

			if (NULL != g_pDirectDraw7)
			{
				g_pDirectDraw7->Release();
				g_pDirectDraw7 = NULL;
			}
		}
	}
	g_csDDraw.Unlock();
}


CDDrawRenderObject::CDDrawRenderObject():
m_bEnable(true)
,m_bStretch(false)
,m_nWidth(0)
,m_nHeight(0)
,m_pOffscreenSurface(NULL)
,m_hPlayWnd(NULL)
{
	InitDDrawGlobal();
}

CDDrawRenderObject::~CDDrawRenderObject()
{
	DeinitDDrawGlobal();
}

int CDDrawRenderObject::init( int nWidth,int nHeight )
{
	HRESULT hr;
	DDSURFACEDESC2	ddsd;
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
	ddsd.dwWidth = nWidth;
	ddsd.dwHeight = nHeight;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_LIVEVIDEO /*| DDSCAPS_SYSTEMMEMORY*/| DDSCAPS_VIDEOMEMORY ;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC | DDPF_YUV | DDPF_COMPRESSED;
	ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V','1','2');
	ddsd.ddpfPixelFormat.dwYUVBitCount = 12;

	m_csOffScreenSurface.Lock();
	hr = g_pDirectDraw7->CreateSurface(&ddsd,&m_pOffscreenSurface,NULL);
	if (FAILED(hr))
	{
		m_pOffscreenSurface = NULL;
		m_csOffScreenSurface.Unlock();
		return -1;
	}
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_csOffScreenSurface.Unlock();


	return 0;
}

int CDDrawRenderObject::deinit()
{
	m_csOffScreenSurface.Lock();
	if (NULL == m_pOffscreenSurface)
	{
		m_csOffScreenSurface.Unlock();
		return 0;
	}

	m_pOffscreenSurface->Release();
	m_pOffscreenSurface = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_csOffScreenSurface.Unlock();


	return 0;
}

int CDDrawRenderObject::setRenderWnd( HWND wnd )
{
	m_csPlayWnd.Lock();
	m_hPlayWnd = wnd;
	m_csPlayWnd.Unlock();
	return 0;
}

int CDDrawRenderObject::render( char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride )
{
	unsigned int i;
	m_csOffScreenSurface.Lock();
	if (NULL == m_pOffscreenSurface)
	{
		m_csOffScreenSurface.Unlock();
		return 0;
	}

	if (0 == nWidth || 0 == nHeight)
	{
		m_csOffScreenSurface.Unlock();
		return 0;
	}

	if (nWidth != m_nWidth || nHeight != m_nHeight 
		&& (0 != nWidth && 0 != nHeight))
	{
		deinit();
	}

	if (NULL == m_pOffscreenSurface)
	{
		if (0 != init(nWidth,nHeight))
		{
			m_csOffScreenSurface.Unlock();
			return -1;
		}
	}

	const unsigned char *pY = (unsigned char *)pYData;
	const unsigned char *pU = (unsigned char *)pUData;
	const unsigned char *pV = (unsigned char *)pVData;

	unsigned int width    = nWidth;
	unsigned int height   = nHeight;
	unsigned int yStride  = nYStride;
	unsigned int uvStride = nUVStride;

	DDSURFACEDESC2 ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	HRESULT hr;
	hr = m_pOffscreenSurface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);
	if (FAILED(hr))
	{
		switch(hr)
		{
		case DDERR_SURFACELOST:
			g_pDirectDraw7->RestoreAllSurfaces();
			break;
		case DDERR_SURFACEBUSY:
			m_pOffscreenSurface->Unlock(NULL);
			break;
		default:
			break;
		}
		return -1;
	}

	PBYTE pDestY = (PBYTE)ddsd.lpSurface;
	PBYTE pDestV = (PBYTE)ddsd.lpSurface + ddsd.lPitch * ddsd.dwHeight;
	PBYTE pDestU = (PBYTE)ddsd.lpSurface + ddsd.lPitch * ddsd.dwHeight*5/4;
	PBYTE pDestYBack = pDestY;                      
	PBYTE pDestUBack = pDestU;			  
	PBYTE pDestVBack = pDestV;

	for (i = 0;i<height;i++)
	{
		memcpy(pDestY,pY,yStride);
		pDestY += ddsd.lPitch;
		pY += yStride;
	}

	for (i = 0;i<height/2;i++)
	{
		memcpy(pDestU,pU,uvStride);
		pDestU += ddsd.lPitch/2;
		pU += uvStride;
	}

	for (i = 0;i<height/2;i++)
	{
		memcpy(pDestV,pV,uvStride);
		pDestV += ddsd.lPitch/2;
		pV += uvStride;
	}

	hr = m_pOffscreenSurface->Unlock(NULL);

	m_csPlayWnd.Lock();
	if (m_hPlayWnd != NULL)
	{
		if(m_bStretch)
		{
			RECT rcDsp;
			::GetClientRect( m_hPlayWnd,&rcDsp);
			POINT ptTL;
			POINT ptRB;

			int h = rcDsp.bottom - rcDsp.top;
			int w = rcDsp.right - rcDsp.left;
			int nDispWidth,nDispHeight;

			if (w * m_nHeight > h * m_nWidth)  //满高 宽度变化
			{	
				nDispHeight = h;
				nDispWidth = h * nWidth / nHeight;
				ptTL.x = rcDsp.left + (w - nDispWidth) / 2;
				ptTL.y = rcDsp.top + 1;
				ptRB.x = rcDsp.right - (w - nDispWidth ) / 2;
				ptRB.y = rcDsp.bottom - 1;
			}

			if (w * m_nHeight <= h * m_nWidth)  //满宽  高度变化 
			{
				nDispWidth = w;
				nDispHeight = nDispWidth * nHeight / nWidth;
				ptTL.x = rcDsp.left + 1;
				ptTL.y = rcDsp.top + (h - nDispHeight) / 2;
				ptRB.x = rcDsp.right - 1;
				ptRB.y = rcDsp.bottom - (h - nDispHeight) / 2;
			}

			::ClientToScreen( m_hPlayWnd,&ptTL);
			::ClientToScreen( m_hPlayWnd,&ptRB);
			SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);

			RECT rcSrc;
			SetRect(&rcSrc,0,0,width,height);
			g_csClipper.Lock();
			g_pClipper->SetHWnd(NULL,m_hPlayWnd);
			if (::IsWindowVisible(m_hPlayWnd))
			{
				g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
			}
			g_csClipper.Unlock();
		}else
		{
			RECT rcDsp;
			::GetClientRect( m_hPlayWnd,&rcDsp);
			POINT ptTL;
			POINT ptRB;
			ptTL.x = rcDsp.left + 1;
			ptTL.y = rcDsp.top + 1;
			ptRB.x = rcDsp.right - 1;
			ptRB.y = rcDsp.bottom - 1;
			::ClientToScreen( m_hPlayWnd,&ptTL);
			::ClientToScreen( m_hPlayWnd,&ptRB);
			SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);

			RECT rcSrc;
			SetRect(&rcSrc,0,0,width,height);
			g_csClipper.Lock();
			g_pClipper->SetHWnd(NULL,m_hPlayWnd);
			if (::IsWindowVisible(m_hPlayWnd))
			{
				g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
			}
			g_csClipper.Unlock();
		}
	}
	m_csPlayWnd.Unlock();

	m_csOffScreenSurface.Unlock();
	
	return 0;
}

int CDDrawRenderObject::enableStretch( bool bEnable )
{
	m_bStretch = bEnable;
	return 0;
}

int CDDrawRenderObject::enable( bool bEnable )
{
	m_bEnable = bEnable;
	return 0;
}

