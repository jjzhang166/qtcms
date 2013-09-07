#pragma once
#include "ivideorender.h"
#include <QMutex>
#include <SDL.h>

class CSDLRender :
	public IVideoRender
{
public:
	CSDLRender(void);
	~CSDLRender(void);

public:
	virtual ErrorCode Init(int nWidth,int nHeight);
	virtual ErrorCode DeInit();
	virtual ErrorCode SetRenderWnd(WId qWid);
	virtual ErrorCode Render(char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride);
	virtual bool Enable(bool bEnable);
	virtual void SetRecStatus(bool bRec);
	
public:
	static int InitGlobalResource(WId qWid);
	static int DeInitGlobalResource();
// 	SDL_Surface* ps_screen;
// 	SDL_Overlay* ps_bmp;
	SDL_Rect     d_rect;
	SDL_Rect     s_rect;
	QMutex               m_csWnd;
	WId                 m_hPlayWnd;
};

