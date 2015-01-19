// DDrawRenderObject.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DDrawRenderObject.h"
#include "DDMutex.h"

//CDDMutex				g_csClipper;
CDDMutex                g_csClipper;
LPDIRECTDRAW7           g_pDirectDraw7 = NULL;
LPDIRECTDRAWSURFACE7    g_pPrimarySurface = NULL;	
LPDIRECTDRAWCLIPPER	    g_pClipper = NULL;
bool                    g_bDDrawInit = false;
int						g_nDDrawRef = 0;

void InitDDrawGlobal()
{
	g_csClipper.Lock();
	if (g_nDDrawRef > 0)
	{
		g_nDDrawRef ++;
		g_csClipper.Unlock();
		return;
	}

	// InitSource
	HRESULT hr;
	hr = DirectDrawCreateEx(NULL,(LPVOID *)&g_pDirectDraw7,IID_IDirectDraw7,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7 = NULL;
		g_csClipper.Unlock();
		return;
	}

	hr = g_pDirectDraw7->SetCooperativeLevel(NULL,DDSCL_NORMAL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csClipper.Unlock();
		return ;
	}

	hr = g_pDirectDraw7->CreateClipper(0,&g_pClipper,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_csClipper.Unlock();
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
		g_csClipper.Unlock();
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
		g_csClipper.Unlock();
		return ;
	}

	// ref
	g_nDDrawRef ++;
	g_csClipper.Unlock();
}

void DeinitDDrawGlobal()
{
	g_csClipper.Lock();
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
	g_csClipper.Unlock();
}


CDDrawRenderObject::CDDrawRenderObject():
m_bEnable(true)
,m_bStretch(true)
,m_nWidth(0)
,m_nHeight(0)
,m_nRectStartX(0)
,m_nRectStartY(0)
,m_nRectEndX(0)
,m_nRectEndY(0)
,m_pOffscreenSurface(NULL)
,m_pOffOsdScreenSurface(NULL)
,m_hPlayWnd(NULL)
,m_hExtendWnd(NULL)
{
	InitDDrawGlobal();
}

CDDrawRenderObject::~CDDrawRenderObject()
{
	DeinitDDrawGlobal();
	deinit();
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

	//创建OSD表面
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
	ddsd.dwWidth=nWidth;
	ddsd.dwHeight=nHeight;
	m_nRectSurfaceWidth=nWidth;
	m_nRectSurfaceHeight=nHeight;
	m_csOffOsdScreenSurface.Lock();
	hr=g_pDirectDraw7->CreateSurface(&ddsd,&m_pOffOsdScreenSurface,NULL);
	if (hr!=DD_OK)
	{
		m_pOffOsdScreenSurface=NULL;
		m_csOffOsdScreenSurface.Unlock();
		m_csOffScreenSurface.Lock();
		m_pOffscreenSurface->Release();
		m_pOffscreenSurface = NULL;
		m_csOffScreenSurface.Unlock();
		return -1;
	}
	m_csOffOsdScreenSurface.Unlock();
	return 0;
}

int CDDrawRenderObject::deinit()
{
	m_csOffScreenSurface.Lock();
	if (NULL == m_pOffscreenSurface)
	{
		m_csOffScreenSurface.Unlock();
	}else{
		m_pOffscreenSurface->Release();
		m_pOffscreenSurface = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
		m_csOffScreenSurface.Unlock();
	}
	m_csOffOsdScreenSurface.Lock();
	if (NULL==m_pOffOsdScreenSurface)
	{
		m_csOffOsdScreenSurface.Unlock();
	}else{
		m_pOffOsdScreenSurface->Release();
		m_pOffOsdScreenSurface=NULL;
		m_csOffOsdScreenSurface.Unlock();
	}
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
	m_csOffScreenSurface.Unlock();
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
	m_csOffScreenSurface.Lock();
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
		m_csOffScreenSurface.Unlock();
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

	m_csExtendWnd.Lock();
	if (NULL!=m_hExtendWnd)
	{
		// 存在电子放大的屏幕
		m_csPlayWnd.Lock();
		if (NULL!=m_hPlayWnd)
		{
			//在原来的屏幕中放大图像，不考虑 原始比例显示 

			//step 1:在原来的窗口中放大视频
			RECT rcDsp;
			::GetClientRect(m_hPlayWnd,&rcDsp);
			POINT ptTL;
			POINT ptRB;
			ptTL.x=rcDsp.left+1;
			ptTL.y=rcDsp.top+1;
			ptRB.x=rcDsp.right-1;
			ptRB.y=rcDsp.bottom-1;
			::ClientToScreen(m_hPlayWnd,&ptTL);
			::ClientToScreen(m_hPlayWnd,&ptRB);
			SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);

			//设置放大区域
			RECT rcSrc;
			setZoomRect(rcSrc,width,height);
			g_csClipper.Lock();
			g_pClipper->SetHWnd(NULL,m_hPlayWnd);
			if (::IsWindowVisible(m_hPlayWnd))
			{
				g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
			}else{
				//do nothing
			}
			g_csClipper.Unlock();

			//step 2:在额外的窗口中 渲染 原图像+矩形
			{
				RECT rcDsp;
				::GetClientRect(m_hExtendWnd,&rcDsp);
				POINT ptTL;
				POINT ptRB;
				ptTL.x=rcDsp.left+1;
				ptTL.y=rcDsp.top+1;
				ptRB.x=rcDsp.right-1;
				ptRB.y=rcDsp.bottom-1;
				::ClientToScreen(m_hExtendWnd,&ptTL);
				::ClientToScreen(m_hExtendWnd,&ptRB);
				SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);

				RECT rcSrc;
				SetRect(&rcSrc,0,0,width,height);
				g_csClipper.Lock();
				g_pClipper->SetHWnd(NULL,m_hExtendWnd);

				if (::IsWindowVisible(m_hExtendWnd))
				{
					m_csOffOsdScreenSurface.Lock();
					hr=m_pOffOsdScreenSurface->Blt(&rcSrc,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,NULL);
					if (DD_OK!=hr)
					{
						g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);
					}else{
						HDC tHdc=NULL;
						hr=m_pOffOsdScreenSurface->GetDC(&tHdc);
						if (DD_OK==hr&&NULL!=tHdc)
						{
							DrawARectangle(tHdc);
							m_pOffOsdScreenSurface->ReleaseDC(tHdc);
							g_pPrimarySurface->Blt(&rcDsp,m_pOffOsdScreenSurface,&rcSrc,DDBLT_WAIT,0);
						}else{
							//do nothing
						}
					}
					m_csOffOsdScreenSurface.Unlock();
				}else{
					//do nothing
				}
				g_csClipper.Unlock();
			}
		}else{
			//do nothing
		}
		m_csPlayWnd.Unlock();
	}else{
		//不存在电子放大
		m_csPlayWnd.Lock();
		if (NULL!=m_hPlayWnd)
		{
			if (!m_bStretch)
			{
				RECT rcDsp;
				::GetClientRect(m_hPlayWnd,&rcDsp);
				POINT ptTL;
				POINT ptRB;
				int h=rcDsp.bottom-rcDsp.top;
				int w=rcDsp.right-rcDsp.left;
				int nDispWidth,nDispHeight;
				if (w*m_nHeight>h*m_nWidth)//满高 宽度变化
				{
					nDispHeight=h;
					nDispWidth=h*nWidth/nHeight;
					ptTL.x=rcDsp.left+(w-nDispWidth)/2;
					ptTL.y=rcDsp.top+1;
					ptRB.x=rcDsp.right-(w-nDispWidth)/2;
					ptRB.y=rcDsp.bottom-1;
				}else{
					nDispWidth = w;
					nDispHeight = nDispWidth * nHeight / nWidth;
					ptTL.x = rcDsp.left + 1;
					ptTL.y = rcDsp.top + (h - nDispHeight) / 2;
					ptRB.x = rcDsp.right - 1;
					ptRB.y = rcDsp.bottom - (h - nDispHeight) / 2;
				}
				::ClientToScreen(m_hPlayWnd,&ptTL);
				::ClientToScreen(m_hPlayWnd,&ptRB);
				SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);

				RECT rcSrc;
				//SetRect(&rcSrc,0,0,width,height);
				setZoomRect(rcSrc,width,height);
				g_csClipper.Lock();
				g_pClipper->SetHWnd(NULL,m_hPlayWnd);
				if (::IsWindowVisible(m_hPlayWnd))
				{
					g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
				}else{
					//do nothing
				}
				g_csClipper.Unlock();
			}else{
				RECT rcDsp;
				::GetClientRect(m_hPlayWnd,&rcDsp);
				POINT ptTL;
				POINT ptRB;
				ptTL.x=rcDsp.left+1;
				ptTL.y=rcDsp.top+1;
				ptRB.x=rcDsp.right-1;
				ptRB.y=rcDsp.bottom-1;
				::ClientToScreen(m_hPlayWnd,&ptTL);
				::ClientToScreen(m_hPlayWnd,&ptRB);
				SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);
				RECT rcSrc;
				//SetRect(&rcSrc,0,0,width,height);
				setZoomRect(rcSrc,width,height);
				g_csClipper.Lock();
				g_pClipper->SetHWnd(NULL,m_hPlayWnd);
				if (::IsWindowVisible(m_hPlayWnd))
				{
					g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
				}else{
					//do nothing
				}
				g_csClipper.Unlock();
			}
		}else{
			//do nothing
		}
		m_csPlayWnd.Unlock();
	}
	m_csExtendWnd.Unlock();
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

bool CDDrawRenderObject::addExtendWnd( HWND wnd,const char* sName )
{
	m_csExtendWnd.Lock();
	m_hExtendWnd=wnd;
	m_csExtendWnd.Unlock();
	return true;
}

void CDDrawRenderObject::setRenderRect( int nStartX,int nStartY,int nEndX,int nEndY )
{
	m_nRectStartX=nStartX;
	m_nRectStartY=nStartY;
	m_nRectEndX=nEndX;
	m_nRectEndY=nEndY;
	return;
}

void CDDrawRenderObject::removeExtendWnd( const char* sName )
{
	m_csExtendWnd.Lock();
	m_hExtendWnd=NULL;
	m_csExtendWnd.Unlock();
	return;
}

void CDDrawRenderObject::setRenderRectPen( int nLineWidth,int nR,int nG,int nB )
{
	return;
}

void CDDrawRenderObject::DrawARectangle( HDC hdc)
{
	HPEN hpen, hpenOld;

	// Create a green pen.
	int nPenWidth=m_nRectSurfaceWidth/400;

	hpen = CreatePen(PS_SOLID, nPenWidth, RGB(0, 255, 0));
	// Create a red brush.
	int nStartX;
	int nStartY;
	int nEndX;
	int nEndY;
	if (m_nRectStartX<m_nRectEndX)
	{
		nStartX=m_nRectStartX;
		nEndX=m_nRectEndX;
	}else{
		nStartX=m_nRectEndX;
		nEndX=m_nRectStartX;
	}
	if(m_nRectStartY<m_nRectEndY){
		nStartY=m_nRectStartY;
		nEndY=m_nRectEndY;
	}else{
		nStartY=m_nRectEndY;
		nEndY=m_nRectStartY;
	}


	// Select the new pen and brush, and then draw.
	hpenOld =(HPEN) SelectObject(hdc, hpen);

	POINT tOldPoint;
	if (m_nRectSurfaceWidth==0||m_nRectSurfaceHeight==0)
	{
		//do nothing
	}else{
		RECT tExtendWndRect;
		::GetClientRect(m_hExtendWnd,&tExtendWndRect);
		nStartX=nStartX*m_nRectSurfaceWidth/tExtendWndRect.right;
		nEndX=nEndX*m_nRectSurfaceWidth/tExtendWndRect.right;
		nStartY=nStartY*m_nRectSurfaceHeight/tExtendWndRect.bottom;
		nEndY=nEndY*m_nRectSurfaceHeight/tExtendWndRect.bottom;
	}
	MoveToEx(hdc,nStartX,nStartY,&tOldPoint);
	LineTo(hdc,nEndX,nStartY);
	LineTo(hdc,nEndX,nEndY);
	LineTo(hdc,nStartX,nEndY);
	LineTo(hdc,nStartX,nStartY);
	// Do not forget to clean up.
	SelectObject(hdc, hpenOld);
	DeleteObject(hpen);


}

void CDDrawRenderObject::setZoomRect( RECT &tRect,int nWidth,int nHeight )
{
	if (m_nRectEndX==m_nRectStartX||m_nRectStartY==m_nRectEndY)
	{
		SetRect(&tRect,0,0,nWidth,nHeight);
	}else{
		RECT tExtendWndRect;
		if (m_hExtendWnd==NULL)
		{
			tExtendWndRect=m_nLastExtendWndRect;
		}else{
			::GetClientRect(m_hExtendWnd,&tExtendWndRect);
			m_nLastExtendWndRect=tExtendWndRect;
		}
		
		int nStartX;
		int nStartY;
		int nEndX;
		int nEndY;
		if (m_nRectStartX<m_nRectEndX)
		{
			nStartX=m_nRectStartX;
			nEndX=m_nRectEndX;
		}else{
			nStartX=m_nRectEndX;
			nEndX=m_nRectStartX;
		}
		if(m_nRectStartY<m_nRectEndY){
			nStartY=m_nRectStartY;
			nEndY=m_nRectEndY;
		}else{
			nStartY=m_nRectEndY;
			nEndY=m_nRectStartY;
		}
		if (tExtendWndRect.right==0||tExtendWndRect.bottom==0)
		{
			SetRect(&tRect,0,0,nWidth,nHeight);
		}else{
			tRect.left=nWidth*nStartX/tExtendWndRect.right;
			tRect.right=nWidth*nEndX/tExtendWndRect.right;
			tRect.top=nHeight*nStartY/tExtendWndRect.bottom;
			tRect.bottom=nHeight*nEndY/tExtendWndRect.bottom;
		}
	}
}

