// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DDRAWRENDEROBJECT_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// DDRAWRENDEROBJECT_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef __DDRAWRENDEROBJECT_HEAD_FILE_SANDU9YS8B9S8__
#define __DDRAWRENDEROBJECT_HEAD_FILE_SANDU9YS8B9S8__
#include <platform/IDDrawRender.h>
#include <ddraw.h>
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"ddraw.lib")
#include "DDMutex.h"

class CDDrawRenderObject : public IDDrawRender
{
public:
	CDDrawRenderObject();
	~CDDrawRenderObject();


	virtual int init( int nWidth,int nHeight );

	virtual int deinit();

	virtual int setRenderWnd( HWND wnd );

	virtual int render( char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride );

	virtual int enableStretch( bool bEnable );

	virtual int enable( bool bEnable );

	virtual bool addExtendWnd(HWND wnd,const char* sName);//添加额外渲染的窗口
	virtual void setRenderRect(int nStartX,int nStartY,int nEndX,int nEndY);//设置画矩形的坐标 
	virtual void drawRectToOriginalWnd( int nX,int nY,int nEndX,int nEndY );
	virtual void removeExtendWnd(const char* sName);//移出指定名字的额外渲染窗口
	virtual void setRenderRectPen(int nLineWidth,int nR,int nG,int nB);//设置画矩形的线宽和颜色
private:
	void DrawARectangle(HDC hdc,int nRectStartX,int nRectStartY,int nRectEndX,int nRectEndY,HWND WND) ;
	void setZoomRect(RECT &tRect,int nWidth,int nHeight);
	void checkHr(HRESULT hr);
private:
	bool                 m_bEnable;
	bool                 m_bStretch;
	int                  m_nWidth;
	int                  m_nHeight;
	LPDIRECTDRAWSURFACE7 m_pOffscreenSurface;
	LPDIRECTDRAWSURFACE7 m_pOffOsdScreenSurface;
	HWND                 m_hPlayWnd;
	CDDMutex             m_csPlayWnd;
	CDDMutex             m_csOffScreenSurface;
	CDDMutex             m_csOffOsdScreenSurface;

	HWND				 m_hExtendWnd;
	CDDMutex			 m_csExtendWnd;

	int m_nRectStartX;
	int m_nRectStartY;
	int m_nRectEndX;
	int m_nRectEndY;
	int m_nRectSurfaceWidth;
	int m_nRectSurfaceHeight;
	RECT m_nLastExtendWndRect;

	int m_nOriginalRectStartX;
	int m_nOriginalRectStartY;
	int m_nOriginalRectEndX;
	int m_nOriginalRectEndY;
};

#endif