#include "DDrawRender.h"

LPDIRECTDRAW7				g_pDirectDraw7 = NULL;
LPDIRECTDRAWSURFACE7			g_pPrimarySurface = NULL;
LPDIRECTDRAWCLIPPER			g_pClipper = NULL;
bool							g_bDDrawInit = false;
QMutex						g_csDDrawInit;
QMutex						g_csClipper;
CDDrawRender::CDDrawRender(void)
{
	m_pOffscreenSurface = NULL;
	m_bInit = false;
	m_bEnable = true;
	m_bRecStatus = false;
	m_nWidth = 0;
	m_nHeight = 0;
}

CDDrawRender::~CDDrawRender(void)
{
}

IVideoRender::ErrorCode CDDrawRender::Init(int nWidth,int nHeight)
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

	hr = g_pDirectDraw7->CreateSurface(&ddsd,&m_pOffscreenSurface,NULL);
	if (FAILED(hr))
	{
		m_pOffscreenSurface = NULL;
		return E_FAILED;
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_bInit = true;
	return SUCCESS;
}

IVideoRender::ErrorCode CDDrawRender::DeInit()
{
	if (!m_bInit)
	{
		return SUCCESS;
	}

	if(NULL != m_pOffscreenSurface)
	{
		m_pOffscreenSurface->Release();
		m_pOffscreenSurface = NULL;
	}
	m_nWidth = 0;
	m_nHeight = 0;
	m_bInit = false;
	return SUCCESS;
}
	
IVideoRender::ErrorCode CDDrawRender::Render(char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride)
{	
	qDebug("nwidth£º%d\nnheight£º%d\n",nWidth,nHeight);
	if (!m_bEnable)
	{
		return SUCCESS;
	}
	if (nWidth != m_nWidth || nHeight != m_nHeight
		&& (0 != nWidth && 0 != nHeight))
	{
		DeInit();
	}

	if (!m_bInit)
	{
		if (SUCCESS != Init(nWidth,nHeight))
		{
			return E_NOT_INIT;
		}
	}

	DDSURFACEDESC2 ddsd;

	const HI_U8 *pY = (unsigned char *)pYData;
	const HI_U8 *pU = (unsigned char *)pUData;
	const HI_U8 *pV = (unsigned char *)pVData;
	HI_U32 width = nWidth;
	HI_U32 height = nHeight;
	HI_U32 yStride = nYStride;
	HI_U32 uvStride = nUVStride;
	if (0 == nWidth || 0 == nHeight)
	{
		return SUCCESS;
	}
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
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
		return E_LOCK_FAILED;
	}

	PBYTE pDestY = (PBYTE)ddsd.lpSurface;
	PBYTE pDestV = (PBYTE)ddsd.lpSurface + ddsd.lPitch * ddsd.dwHeight;
	PBYTE pDestU = (PBYTE)ddsd.lpSurface + ddsd.lPitch * ddsd.dwHeight*5/4;
	PBYTE pDestYBack = pDestY;                      
	PBYTE pDestUBack = pDestU;			  
	PBYTE pDestVBack = pDestV;			  
	HI_U32 i;
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

	m_csWnd.lock();
	if (m_hPlayWnd != NULL)
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
		g_csClipper.lock();
		g_pClipper->SetHWnd(NULL,m_hPlayWnd);
		if (::IsWindowVisible(m_hPlayWnd))
		{
			g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
		}
		g_csClipper.unlock();
	}
	m_csWnd.unlock();
	return SUCCESS;
}

bool CDDrawRender::Enable(bool bEnable)
{
	bool bRet = m_bEnable;
	m_bEnable = bEnable;
	return bRet;
}

void CDDrawRender::SetRecStatus(bool bRec)
{
	m_bRecStatus = bRec;
}

IVideoRender::ErrorCode CDDrawRender::SetRenderWnd(WId hPlayWnd)
{
	m_csWnd.lock();
	m_hPlayWnd = hPlayWnd;
	m_csWnd.unlock();
	return SUCCESS;
}

int CDDrawRender::InitGlobalResource(WId hWnd)
{
	g_csDDrawInit.lock();
	if (g_bDDrawInit)
	{
		g_csDDrawInit.unlock();
		return 0;
	}

	HRESULT hr;
	hr = DirectDrawCreateEx(NULL,(LPVOID *)&g_pDirectDraw7,IID_IDirectDraw7,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7 = NULL;
		g_csDDrawInit.unlock();
		return -1;
	}

	hr = g_pDirectDraw7->SetCooperativeLevel(NULL,DDSCL_NORMAL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csDDrawInit.unlock();
		return -1;
	}

	hr = g_pDirectDraw7->CreateClipper(0,&g_pClipper,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csDDrawInit.unlock();
		return -1;
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
		g_csDDrawInit.unlock();
		return -1;
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
		g_csDDrawInit.unlock();
		return -1;
	}
	g_bDDrawInit = true;

	g_csDDrawInit.unlock();

	return 0;
}

int CDDrawRender::DeinitGlobalResource()
{
	g_csDDrawInit.lock();
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

	g_bDDrawInit = false;
	g_csDDrawInit.unlock();
	return 0;
}