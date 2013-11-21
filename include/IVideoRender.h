#ifndef __IVIDEORENDER_HEAD_FILE_ASDN8998YGF780AGW__
#define __IVIDEORENDER_HEAD_FILE_ASDN8998YGF780AGW__
#include <libpcom.h>
#include <QtGui/QWidget>

interface IVideoRender : public IPComBase
{
	virtual int init(int nWidth,int nHeight) = 0;
	virtual int deinit() = 0;
	virtual int setRenderWnd(QWidget * wnd) = 0;
	virtual int render(char *data,char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride,int lineStride,const QString & pixelFormat,int flags) = 0;
	virtual int enable(bool bEnable) = 0;
	virtual int enableStretch(bool bEnable) = 0;
	virtual bool isRenderEnable() = 0;
	virtual bool isStretchEnable() = 0;
	virtual bool isPixelFormatAvalible(const QString &sFormat) = 0;
};

#endif