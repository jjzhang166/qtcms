#include "FfmpegH264Dec.h"
#include <QtOpenGL/QGLWidget>
CAvLibDll avlib_dll;
QMutex g_csInitAvlib;
CFfmpegH264Dec::CFfmpegH264Dec(void)
{
	m_hDec = 0;
	m_nVideoWidth = 0;
	m_nVideoHeight = 0;
	m_bInit = false;
	m_videoBuf = NULL;
	m_CurRender = IVideoRender::CreateRender(IVideoRender::RT_SDL);
}

CFfmpegH264Dec::~CFfmpegH264Dec(void)
{
	g_csInitAvlib.lock();
	DeInit();
	g_csInitAvlib.unlock();
}

void CFfmpegH264Dec::SetRenderWnd(WId qWnd)
{
	m_CurRender->SetRenderWnd(qWnd);
}

void CFfmpegH264Dec::Init(int nWidth,int nHeight)
{
	m_hDec = avlib_dll.CreateVideoDecoder();
	if (0 == m_hDec)
	{
		return;
	}
	int ret = avlib_dll.VideoDecoderInit(m_hDec,AV_CODEC_ID_H264,nWidth,nHeight);
	if (0 != ret)
	{
		return;
	}
	if (m_videoBuf != NULL)
	{
		delete m_videoBuf;
		m_videoBuf = NULL;
	}

	m_videoBuf = new unsigned char[nWidth*nHeight*2];
	m_nVideoWidth = nWidth;
	m_nVideoHeight = nHeight;
	m_bInit = true;
}

void CFfmpegH264Dec::DeInit()
{
	//m_csDecInit.lock();
	if (0 != avlib_dll.VideoDeocderRelease(m_hDec))
	{
		m_csDecInit.unlock();
	}
	m_hDec = 0;
	if (m_videoBuf != NULL)
	{
		delete m_videoBuf;
		m_videoBuf = NULL;
	}

	m_nVideoWidth = 0;
	m_nVideoHeight = 0;

	m_bInit = false;
	//m_csDecInit.unlock();
}
extern int GetWidthHeight(char *stream,int stream_len,int *width,int *height);
void CFfmpegH264Dec::Decode(LPVOID pData,int nDataLen)
{
	m_csDecInit.lock();
	int nWidth = 0;
	int nHeight = 0;
	GetWidthHeight((char *)pData,nDataLen,&nWidth,&nHeight);

	if ((nWidth != m_nVideoWidth ||
		nHeight != m_nVideoHeight) &&
		(0 != nWidth && 0 != nHeight))
	{
		g_csInitAvlib.lock();
		DeInit();
		g_csInitAvlib.unlock();
	}

	if(!m_bInit)
	{	
		if (0 == nWidth ||
			0 == nHeight)
		{
			m_csDecInit.unlock();
		}
		g_csInitAvlib.lock();
		Init(nWidth,nHeight);
		g_csInitAvlib.unlock();
	}

	int outsize = 0;
	FrameData temp;	
	memset(&temp,0,sizeof(FrameData));

	g_csInitAvlib.lock();
	avlib_dll.VideoDecoderDecode(m_hDec,(unsigned char*)pData,nDataLen,m_videoBuf,&outsize,0,0,&temp);
	g_csInitAvlib.unlock();
	m_csDecInit.unlock();

	qDebug("%d\n%d\n%d\n",temp.pY,temp.pU,temp.pV);
	if (outsize > 0)
	{
		//äÖÈ¾
		if (NULL != m_CurRender)
		{
			m_CurRender->Render((char *)temp.pY,(char *)temp.pU,(char *)temp.pV,temp.nWidth,temp.nHeight,temp.nYStride,temp.nUVStride);
		}
	}
}

void CFfmpegH264Dec::FlushDecode()
{
	m_csDecInit.lock();
	if (!m_bInit)
	{
		m_csDecInit.unlock();
	}
	m_csDecInit.unlock();
}
