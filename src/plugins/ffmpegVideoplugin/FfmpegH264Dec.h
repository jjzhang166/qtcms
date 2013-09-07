#pragma once
#include "AvLibDll.h"
#include <QMutex>
#include "IVideoRender.h"
typedef void* LPVOID;

class CFfmpegH264Dec
{
public:
	CFfmpegH264Dec(void);
	~CFfmpegH264Dec(void);
public:
	void Init(int nWidth,int nHeight);
	void DeInit();
	void Decode(LPVOID pData,int nDataLen);
	void FlushDecode();
	void SetRenderWnd(WId qWnd);
private:
	int m_hDec;
	int m_nVideoWidth;
	int m_nVideoHeight;
	bool m_bInit;
	QMutex m_csDecInit;
public:
	unsigned char * m_videoBuf;
	IVideoRender * m_CurRender;
};

