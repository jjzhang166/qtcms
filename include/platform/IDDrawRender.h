#ifndef __IDDRAWRENDER_HEAD_FILE_SANVBPSPA8UV98SA8D__
#define __IDDRAWRENDER_HEAD_FILE_SANVBPSPA8UV98SA8D__

class IDDrawRender{
public:
	virtual int init( int nWidth,int nHeight ) = 0;
	virtual int deinit() = 0;
	virtual int setRenderWnd(HWND wnd) = 0;
	virtual int render(char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride) = 0;
	virtual int enableStretch( bool bEnable ) = 0;
	virtual int enable( bool bEnable ) = 0;
};

#endif