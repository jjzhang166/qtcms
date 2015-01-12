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

	virtual bool addExtendWnd(void *pWnd,const char* sName)=0;//添加额外渲染的窗口
	virtual void setRenderRect(int nX,int nY,int nWidth,int nHeight)=0;//设置画矩形的坐标，nX,nY 为屏幕的绝对坐标
	virtual void removeExtendWnd(const char* sName)=0;//移出指定名字的额外渲染窗口
	virtual void setRenderRectPen(int nLineWidth,int nR,int nG,int nB)=0;//设置画矩形的线宽和颜色

};

#endif