#pragma once
#include <ddraw.h>
#include <QMutex>
#include "ivideorender.h"
#include <QtGui/QWidget>
typedef unsigned char   HI_U8;
typedef unsigned long   HI_U32;
class CDDrawRender :
	public IVideoRender
{
public:
	CDDrawRender(void);
	~CDDrawRender(void);

public:
	virtual ErrorCode Init(int nWidth,int nHeight);
	virtual ErrorCode DeInit();
	virtual ErrorCode Render(char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride);
	virtual ErrorCode SetRenderWnd(WId hPlayWnd);
	virtual bool Enable(bool bEnable);
	virtual void SetRecStatus(bool bRec);

public:
	static int InitGlobalResource(WId hWnd);
	static int DeinitGlobalResource();

private:
	LPDIRECTDRAWSURFACE7 m_pOffscreenSurface;
	bool                 m_bInit;
	bool                 m_bEnable;
	QMutex               m_csWnd;
	WId                 m_hPlayWnd;
	bool                 m_bRecStatus;
	int m_nWidth;
	int m_nHeight;
};

