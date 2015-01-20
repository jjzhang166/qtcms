#ifndef DDRAWRENDER_H
#define DDRAWRENDER_H
#include "ddrawrender_global.h"
#include <IVideoRender.h>
#include <IVideoRenderDigitalZoom.h>
#include <QtCore/QMutex>
#include <platform/IDDrawRender.h>

class DdrawRender : public IVideoRender,public IVideoRenderDigitalZoom
{
public:
	DdrawRender();
	~DdrawRender();

	virtual int init( int nWidth,int nHeight );

	virtual int deinit();

	virtual int setRenderWnd( QWidget * wnd );

	virtual int render( char *data,char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride,const QString & pixelFormat,int flags );

	virtual int enable( bool bEnable );

	virtual int enableStretch( bool bEnable );

	virtual bool isRenderEnable();

	virtual bool isStretchEnable();

	virtual bool isPixelFormatAvalible( const QString &sFormat );

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();


	virtual bool addExtendWnd(QWidget * wnd,const QString sName);//添加额外渲染的窗口
	virtual void setRenderRect(int nStartX,int nStartY,int nEndX,int nEndY);//设置画矩形的坐标，nX,nY 为屏幕的绝对坐标
	virtual void drawRectToOriginalWnd(int nStartX,int nStartY,int nEndX,int nEndY);
	virtual void removeExtendWnd(const QString sName);//移出指定名字的额外渲染窗口
	virtual void setRenderRectPen(int nLineWidth,int nR,int nG,int nB);//设置画矩形的线宽和颜色

private:
	int m_nRef;
	QMutex m_csRef;
	bool m_bStretch;
	bool m_bEnable;
	IDDrawRender * m_renderObj;

};

#endif // DDRAWRENDER_H
