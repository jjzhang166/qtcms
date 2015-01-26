#ifndef __IVIDEORENDERDIGITALZOOM_HEAD_FILE_ASDN8998YGF780AGW__
#define __IVIDEORENDERDIGITALZOOM_HEAD_FILE_ASDN8998YGF780AGW__
#include <libpcom.h>
#include <QtGui/QWidget>

interface IVideoRenderDigitalZoom : public IPComBase
{
	virtual bool addExtendWnd(QWidget * wnd,const QString sName)=0;//添加额外渲染的窗口
	virtual void setRenderRect(int nStartX,int nStartY,int nEndX,int nEndY,int nWndWidth,int nWndHeight)=0;//nX,nY 开始坐标，ntEndX,inEndY为结束坐标（相对当前的屏幕）
	virtual void drawRectToOriginalWnd(int nX,int nY,int nEndX,int nEndY)=0;//nX,nY 开始坐标，ntEndX,inEndY为结束坐标（相对当前的屏幕）
	virtual void removeExtendWnd(const QString sName)=0;//移出指定名字的额外渲染窗口
	virtual void setRenderRectPen(int nLineWidth,int nR,int nG,int nB)=0;//设置画矩形的线宽和颜色
};

#endif