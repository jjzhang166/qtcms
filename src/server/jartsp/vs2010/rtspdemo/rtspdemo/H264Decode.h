#pragma once
#include "decoder.h"
#include "hi_config.h"
#include "hi_h264api.h"
#include "gfun.h"

#define AVIFRAMERATE 16
typedef void (* DCMSGCALLBACK)(HDC hdc,int nChl,LPVOID lParam);

#define UM_DECMSG_NORES			(WM_USER + 2001)

enum _enDecMsg{
	Msg_No_Resource,

	Msg_Cnt,
};


typedef struct _tagFrameBack{
	HI_U8	*pY;
	HI_U8	*pU;
	HI_U8	*pV;
	HI_U32	uWidth;
	HI_U32	uHeight;
	HI_U32	uYStride;
	HI_U32	uUVStride;
}FrameBack;

class CH264Decode :
	public CDecoder
{
public:
	CH264Decode(void);
	BOOL	InitDec();
	BOOL	DeinitDec();
	void	PreviewRedraw();
	BOOL	SetPreviewWnd(HWND hWnd);
	int		CaptureImage(char *sSavePath);
	
	BOOL	InputDate(LPVOID pDate,int nDateLen);
	BOOL	Flush();
	bool	EnablePreview(bool bEnable);
	BOOL	SetFinishedDCCallBackTo(DCMSGCALLBACK copyDcDate,int nDcChl,LPVOID lParam);

 
private:
	BOOL	InitOffscreenSurface();
	BOOL	PreviewDraw(H264_DEC_FRAME_S DecodeData);
	BOOL	DecodePreviewData(H264_DEC_FRAME_S DecodeData);

private:
    H264_LIBINFO_S						m_DecLibInfo;
 	HI_HDL								m_hDec;
 	int									m_nDcChl;
 
 	CRITICAL_SECTION					m_csDcCallback;
 	FrameBack							m_DecFramesBack;
 	CRITICAL_SECTION					m_csDecFrames;
 
 	bool								m_bPreEnable;
	bool								m_bInit;

    
    HWND                    m_PlayWnd;  
	LPDIRECTDRAWSURFACE7	m_pOffscreenSurface;			                    //offscreen页面
	LPDIRECTDRAWSURFACE7	m_pOSDSurface;					                    //写OSD的页面
	LPDIRECTDRAWSURFACE7	m_pFinSurface;					                    //最终offscreen页面
 	DWORD					m_dwLastError;
 	DCMSGCALLBACK  dccallbackFun;
 	LPVOID lpDcUser;
 	CRITICAL_SECTION						m_csCritial;
public:
	~CH264Decode(void);
	int put_jpeg_yuv420p_file(const char * filename, unsigned char *image, int width, 
									   int height, int quality) ;
};
