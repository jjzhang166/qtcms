#pragma once
#include <QtGui/QWidget>

typedef void* LPVOID;
typedef struct _tagFrameData{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int nWidth;
	int nHeight;
	int nYStride;
	int nUVStride;
}FrameData;

class IVideoRender
{
public:
	IVideoRender(void);
	virtual ~IVideoRender(void);

	typedef enum _enErrorCode{
		SUCCESS,
		E_FAILED,
		E_LOCK_FAILED,
		E_OUT_OF_RANGE,
		E_NOT_INIT,
	}ErrorCode;

	typedef enum _RenderType{
		RT_DDRAW,
		RT_SDL,
		RT_CNT
	}RenderType;

	typedef enum _enEventType{
		EVENT_PRERENDER,
		EVENT_CNT,
	}EventType;

	typedef int (*EventFun)(EventType e,LPVOID pData,int nDataSize,LPVOID pUser);

	typedef struct _tagEventMap{
		EventType e;
		EventFun  proc;
		LPVOID    pUser;
	}EventMap;

public:
	virtual ErrorCode Init(int nWidth,int nHeight) = 0;
	virtual ErrorCode DeInit() = 0;
	virtual ErrorCode SetRenderWnd(WId hPlayWnd) = 0;
	virtual ErrorCode Render(char *pYData,char *pUData,char *pVData,int nWidth,int nHeight,int nYStride,int nUVStride) = 0;
	virtual bool Enable(bool bEnable) = 0;
	virtual void SetRecStatus(bool bRec) = 0;

public:
	// factory
	static IVideoRender * CreateRender(RenderType type);
	friend const RenderType& operator++(RenderType &lhr,int){return lhr = (RenderType)((lhr + 1));};
};

