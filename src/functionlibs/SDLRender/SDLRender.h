#ifndef SDLRENDER_H
#define SDLRENDER_H

#include <QtGui/QWidget>
#include <QtCore/QMutex>
#include <SDL.h>
#include "SDLRender_global.h"
#include <QDebug>
#include "IVideoRender.h"

class SDLRender :
	public IVideoRender
{
public:
	SDLRender();
	~SDLRender();

	virtual int init(int nWidth,int nHeight);
	virtual int deinit();
	virtual int setRenderWnd(QWidget * wnd);
	virtual int render(char *data,char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride,const QString & pixelFormat,int flags);
	virtual int enable(bool bEnable);
	virtual int enableStretch(bool bEnable);
	virtual bool isRenderEnable();
	virtual bool isStretchEnable();
	virtual bool isPixelFormatAvalible(const QString &sFormat);

	virtual QString getModeName();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();
private:
	int m_nRef;
	QMutex m_csRef;

	QWidget* pWidget;
	SDL_Window * m_pWindow;
	SDL_Renderer * m_pRender;
	SDL_Texture * m_pTexture;

};

#endif // SDLRENDER_H
