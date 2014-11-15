#include "StdAfx.h"
#include "H264Decode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define JPEG_QUALITY 50     //它的大小决定jpg的质量好坏



extern    LPDIRECTDRAW7			    g_pDirectDraw7;				                       	//用于预览的DirectDraw对象
extern    LPDIRECTDRAWSURFACE7    	g_pPrimarySurface;	
extern    LPDIRECTDRAWCLIPPER	    g_pClipper;	
extern	  CRITICAL_SECTION		g_csClipper;
extern	  HI_U64				gs_aTimeStamp;

CH264Decode::CH264Decode(void)
{
	memset(&m_DecFramesBack,0,sizeof(m_DecFramesBack));
	m_DecFramesBack.pU = new HI_U8[1280*720];
	m_DecFramesBack.pV = new HI_U8[1280*720];
	m_DecFramesBack.pY = new HI_U8[1280*720];
	m_pOffscreenSurface=NULL;
	m_pOSDSurface = NULL;
	m_pFinSurface = NULL;
 	dccallbackFun=NULL;
	m_bInit = false;
	InitializeCriticalSection(&m_csCritial);
	InitializeCriticalSection(&m_csDcCallback);
	InitializeCriticalSection(&m_csDecFrames);

}


BOOL CH264Decode::InitDec()
{
	HI_S32	hr;
	hr = Hi264DecGetInfo(&m_DecLibInfo);
	if (0 != hr)
	{
		return FALSE;
	}

	H264_DEC_ATTR_S	DecAttr;
	DecAttr.uPictureFormat = 0x00;
	DecAttr.uStreamInType = 0x00;
	DecAttr.uPicWidthInMB = 0x50;
	DecAttr.uPicHeightInMB = 0x2D;
	DecAttr.uBufNum = 2;
	DecAttr.uWorkMode = 0x01;
	DecAttr.pUserData = NULL;
	DecAttr.uReserved = 0;

	m_hDec = Hi264DecCreate(&DecAttr);
	if (NULL == m_hDec)
	{
		return FALSE;
	}

	BOOL bRet = InitOffscreenSurface();
	if (!bRet)
	{
		Hi264DecDestroy(m_hDec);
		m_hDec = NULL;
		return FALSE;
	}

	m_bInit = true;

	return TRUE;

}

BOOL CH264Decode::DeinitDec()
{
	if (!m_bInit)
	{
		return TRUE;
	}
	if (m_pOffscreenSurface!=NULL)
	{
		m_pOffscreenSurface->Release();
		m_pOffscreenSurface = NULL;
	}
	if (m_pOSDSurface != NULL)
	{
		m_pOSDSurface->Release();
		m_pOSDSurface = NULL;
	}
	if (m_pFinSurface != NULL)
	{
		m_pFinSurface->Release();
		m_pFinSurface = NULL;
	}

	if (m_hDec!=NULL)
	{
		Flush();
		Hi264DecDestroy(m_hDec);
		m_hDec = NULL;
	}
	
	m_bInit = FALSE;
	
	
	return TRUE;
}

static long n_packet=0;
BOOL CH264Decode::InputDate(LPVOID pDate,int nDateLen)
{
	EnterCriticalSection(&g_csDecInit);
	if (!m_bInit)
	{
		BOOL bRet = InitDec();
		if (!bRet)
		{
			PostMessage(m_PlayWnd,UM_DECMSG_NORES,Msg_No_Resource,0);
			LeaveCriticalSection(&g_csDecInit);
			return FALSE;
		}
	}
	LeaveCriticalSection(&g_csDecInit);

	HI_S32	nResult;
	H264_DEC_FRAME_S	DecFrames;
	nResult = Hi264DecFrame(m_hDec,(HI_U8 *)pDate,nDateLen,0,&DecFrames,0);
	if (HI_H264DEC_NEED_MORE_BITS == nResult)
	{
		return TRUE;
	}
	else if (HI_H264DEC_ERR_HANDLE == nResult)                 
	{
		return FALSE;
	}
	while (HI_H264DEC_OK == nResult)
	{
		DecodePreviewData(DecFrames);
		EnterCriticalSection(&m_csDecFrames);
		m_DecFramesBack.uHeight = DecFrames.uHeight;
		m_DecFramesBack.uWidth = DecFrames.uWidth;
		m_DecFramesBack.uUVStride = DecFrames.uUVStride;
		m_DecFramesBack.uYStride = DecFrames.uYStride;
		memcpy(m_DecFramesBack.pY,DecFrames.pY,DecFrames.uHeight * DecFrames.uWidth);
		memcpy(m_DecFramesBack.pU,DecFrames.pU,DecFrames.uWidth * DecFrames.uHeight / 4);
		memcpy(m_DecFramesBack.pV,DecFrames.pV,DecFrames.uWidth * DecFrames.uHeight / 4);
		LeaveCriticalSection(&m_csDecFrames);

		nResult = Hi264DecFrame(m_hDec,NULL,0,0,&DecFrames,0);
		if (HI_H264DEC_NEED_MORE_BITS == nResult)
		{
			break;
		}
	}

	return TRUE;
}

BOOL CH264Decode::InitOffscreenSurface()
{
	HRESULT hr;
	DDSURFACEDESC2	ddsd;
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
	ddsd.dwWidth = 1280;
	ddsd.dwHeight = 720;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_LIVEVIDEO /*| DDSCAPS_SYSTEMMEMORY*/| DDSCAPS_VIDEOMEMORY ;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC | DDPF_YUV | DDPF_COMPRESSED;
 	ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V','1','2');
 	ddsd.ddpfPixelFormat.dwYUVBitCount = 12;




	hr = g_pDirectDraw7->CreateSurface(&ddsd,&m_pOffscreenSurface,NULL);
	if (FAILED(hr))
	{
		m_pOffscreenSurface = NULL;
		return FALSE;
	}

	//OSD页面
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN  | DDSCAPS_LIVEVIDEO  | DDSCAPS_SYSTEMMEMORY/*| DDSCAPS_VIDEOMEMORY*/;
	ddsd.dwWidth = 1280;
	ddsd.dwHeight = 720;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwFourCC = 0;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRBitMask = 0x00ff0000;//0x1f << 11;//
	ddsd.ddpfPixelFormat.dwGBitMask = 0x0000ff00;//0x3f << 5;//
	ddsd.ddpfPixelFormat.dwBBitMask = 0x000000ff;//0x1f;//
//	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0;
	
	hr = g_pDirectDraw7->CreateSurface(&ddsd,&m_pOSDSurface,NULL);
	if (FAILED(hr))
	{
		m_pOffscreenSurface->Release();
		m_pOffscreenSurface = NULL;
		m_pOSDSurface = NULL;
		m_dwLastError = GetLastError();
		return FALSE;
	}

	DDCOLORKEY ddcolorkey;
	ddcolorkey.dwColorSpaceLowValue =0x00;
	ddcolorkey.dwColorSpaceHighValue = 0x00;
	m_pOSDSurface->SetColorKey(DDCKEY_SRCBLT,&ddcolorkey);

	
	//最终offscreen页面
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN  | DDSCAPS_LIVEVIDEO | DDSCAPS_VIDEOMEMORY/* | DDSCAPS_SYSTEMMEMORY*/;
	ddsd.dwWidth = 352;
	ddsd.dwHeight = 288;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
	ddsd.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
	ddsd.ddpfPixelFormat.dwBBitMask = 0x000000ff;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0;

	hr = g_pDirectDraw7->CreateSurface(&ddsd,&m_pFinSurface,NULL);
	if (FAILED(hr))
	{
		m_pOffscreenSurface->Release();
		m_pOffscreenSurface = NULL;
		m_pOSDSurface->Release();
		m_pOSDSurface = NULL;
		m_dwLastError = GetLastError();
		return FALSE;
	}
		
	return TRUE;
}

BOOL CH264Decode::SetPreviewWnd(HWND hWnd)
{  

	EnterCriticalSection(&m_csCritial);
	m_PlayWnd=hWnd;
    LeaveCriticalSection(&m_csCritial);
	return TRUE;
}

void CH264Decode::PreviewRedraw()
{
	if (!m_bInit)
	{
		return;
	}
	EnterCriticalSection(&m_csDecFrames);
	H264_DEC_FRAME_S DecFrames;
	DecFrames.pU = m_DecFramesBack.pU;
	DecFrames.pV = m_DecFramesBack.pV;
	DecFrames.pY = m_DecFramesBack.pY;
	DecFrames.uHeight = m_DecFramesBack.uHeight;
	DecFrames.uWidth = m_DecFramesBack.uWidth;
	DecFrames.uUVStride = m_DecFramesBack.uUVStride;
	DecFrames.uYStride = m_DecFramesBack.uYStride;
	PreviewDraw(DecFrames);
	LeaveCriticalSection(&m_csDecFrames);
}

bool CH264Decode::EnablePreview(bool bEnable)
{
	bool temp = m_bPreEnable;
	m_bPreEnable = bEnable;
	return temp;
}

BOOL CH264Decode::PreviewDraw(H264_DEC_FRAME_S DecodeData)
{
	HI_U32 i;

	if (!m_bPreEnable)
	{
		return true;
	}

 	DDSURFACEDESC2 ddsd;
	const HI_U8 *pY = DecodeData.pY;
    const HI_U8 *pU = DecodeData.pU;
    const HI_U8 *pV = DecodeData.pV;
    HI_U32 width    = DecodeData.uWidth;
    HI_U32 height   = DecodeData.uHeight;
    HI_U32 yStride  = DecodeData.uYStride;
    HI_U32 uvStride = DecodeData.uUVStride;
	if (0 == width || 0 == height)
	{
		return true;
	}
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	HRESULT hr;
	hr = m_pOffscreenSurface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);
	if (FAILED(hr))
	{
		m_dwLastError = hr;//GetLastError();
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
		return FALSE;
	}
	
	
	PBYTE pDestY = (PBYTE)ddsd.lpSurface;
	PBYTE pDestV = (PBYTE)ddsd.lpSurface + ddsd.lPitch * ddsd.dwHeight;
	PBYTE pDestU = (PBYTE)ddsd.lpSurface + ddsd.lPitch * ddsd.dwHeight*5/4;
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

    EnterCriticalSection(&m_csCritial);
	if (m_PlayWnd != NULL)
	{
		RECT rcDsp;
		::GetClientRect( m_PlayWnd,&rcDsp);
		POINT ptTL;
		POINT ptRB;
		ptTL.x = rcDsp.left + 1;
		ptTL.y = rcDsp.top + 1;
		ptRB.x = rcDsp.right - 1;
		ptRB.y = rcDsp.bottom - 1;
		::ClientToScreen( m_PlayWnd,&ptTL);
		::ClientToScreen( m_PlayWnd,&ptRB);
		SetRect(&rcDsp,ptTL.x,ptTL.y,ptRB.x,ptRB.y);
		
		//写OSD

/*		HDC		dcTemp;
 		hr = m_pOSDSurface->GetDC(&dcTemp);
		DDSURFACEDESC2 ddsdOsd = {0};
		ddsdOsd.dwSize = sizeof(ddsdOsd);
		m_pOSDSurface->GetSurfaceDesc(&ddsdOsd);

		HBRUSH BackBrush = CreateSolidBrush(RGB(0,0,0));
		RECT rcOsd;
		SetRect(&rcOsd,0,0,ddsdOsd.dwWidth,ddsdOsd.dwHeight);
		FillRect(dcTemp,&rcOsd,BackBrush);
		DeleteObject(BackBrush);

		EnterCriticalSection(&m_csDcCallback);
 		if (NULL != dccallbackFun)
 		{
 			dccallbackFun(dcTemp,m_nDcChl,lpDcUser);
 		}
		LeaveCriticalSection(&m_csDcCallback);
		m_pOSDSurface->ReleaseDC(dcTemp);

		DDSURFACEDESC2 ddsdFin = {0};
		ddsdFin.dwSize = sizeof(ddsdFin);
		m_pFinSurface->GetSurfaceDesc(&ddsdFin);
		RECT rcFin;
		SetRect(&rcFin,0,0,ddsdFin.dwWidth,ddsdFin.dwHeight);

		hr = m_pFinSurface->Blt(&rcFin,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,NULL);
		hr = m_pFinSurface->Blt(&rcFin,m_pOSDSurface,NULL,DDBLT_WAIT | DDBLT_KEYSRC,NULL);*/
		RECT rcSrc;
		SetRect(&rcSrc,0,0,width,height);
		EnterCriticalSection(&g_csClipper);
		g_pClipper->SetHWnd(NULL,m_PlayWnd);
		if (::IsWindowVisible(m_PlayWnd))
		{
			g_pPrimarySurface->Blt(&rcDsp,m_pOffscreenSurface,&rcSrc,DDBLT_WAIT,0);	
		}
		LeaveCriticalSection(&g_csClipper);
	}
	 LeaveCriticalSection(&m_csCritial);

	return TRUE;
}

BOOL CH264Decode::DecodePreviewData(H264_DEC_FRAME_S DecodeData)
{
	PreviewDraw(DecodeData);     
	return TRUE;
}
 
BOOL CH264Decode::Flush()
{
	HI_S32 nResult;
	H264_DEC_FRAME_S	DecFrames;
	if (!m_bInit)
	{
		return FALSE;
	}
	do 
	{
		nResult = Hi264DecFrame(m_hDec,NULL,0,0,&DecFrames,1);
	} while (HI_H264DEC_NO_PICTURE != nResult);
	EnterCriticalSection(&m_csDecFrames);
	m_DecFramesBack.uHeight = 0;
	m_DecFramesBack.uWidth = 0;
	m_DecFramesBack.uUVStride = 0;
	m_DecFramesBack.uYStride = 0;
	LeaveCriticalSection(&m_csDecFrames);
	return TRUE;
}

BOOL CH264Decode::SetFinishedDCCallBackTo(DCMSGCALLBACK copyDcDate,int nChl,LPVOID lParam)
{
	EnterCriticalSection(&m_csDcCallback);
	dccallbackFun=copyDcDate;
	lpDcUser=lParam;
	m_nDcChl = nChl;
	LeaveCriticalSection(&m_csDcCallback);
	return TRUE;
}


CH264Decode::~CH264Decode(void)
{
     DeleteCriticalSection(&m_csCritial);
	 DeleteCriticalSection(&m_csDcCallback);
	 DeleteCriticalSection(&m_csDecFrames);
	 delete m_DecFramesBack.pU;
	 delete m_DecFramesBack.pV;
	 delete m_DecFramesBack.pY;
}

