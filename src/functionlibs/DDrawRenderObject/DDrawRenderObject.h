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

private:
	bool                 m_bEnable;
	bool                 m_bStretch;
	int                  m_nWidth;
	int                  m_nHeight;
	LPDIRECTDRAWSURFACE7 m_pOffscreenSurface;
	HWND                 m_hPlayWnd;
	CDDMutex             m_csPlayWnd;
	CDDMutex             m_csOffScreenSurface;
};

#endif