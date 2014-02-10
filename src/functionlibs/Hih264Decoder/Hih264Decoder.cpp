#include "Hih264Decoder.h"
#include <guid.h>
#include <QDebug>
#include "h264wh.h"

QMutex g_csDecInit;


Hih264Decoder::Hih264Decoder() :
m_nRef(0),
m_nVideoHeight(0),
m_nVideoWidth(0),
m_hDec(NULL),
m_bInit(false)
{

}

Hih264Decoder::~Hih264Decoder()
{

}

int Hih264Decoder::init(int nWidth,int nHeight)
{
	g_csDecInit.lock();

	HiH264DecCreInfo DecAttr;
	DecAttr.uPictureFormat = 0x00;
	DecAttr.uStreamInType = 0x00;
	DecAttr.uPicWidthInMB = nWidth / 16 + ((nWidth % 16) ? 1 : 0);
	DecAttr.uPicHeightInMB = nHeight / 16 + ((nHeight % 16) ? 1 : 0);
	DecAttr.uBufNum = 2;
	DecAttr.uWorkMode = 0x01;
	DecAttr.pUserData = NULL;
	DecAttr.uReserved = 0;

	m_hDec = HiH264Dec_dll.DecCreate(&DecAttr);
	if (NULL == m_hDec)
	{
		g_csDecInit.unlock();
		return 1;
	}
	g_csDecInit.unlock();

	m_nVideoWidth = nWidth;
	m_nVideoHeight = nHeight;

	m_bInit = true;

	return 0;
}
int Hih264Decoder::deinit()
{
	m_csDecInit.lock();
	HiH264Dec_dll.DecDestroy(m_hDec);
	m_hDec = NULL;
	m_nVideoHeight = 0;
	m_nVideoWidth = 0;
	m_bInit = false;
	m_csDecInit.unlock();
	return 0;
}

int Hih264Decoder::decode(char * pData,unsigned int nDataLength)
{

	int nWidth = 0;
	int nHeight = 0;
	GetWidthHeight((char *)pData,nDataLength,&nWidth,&nHeight);
	if ((nWidth != m_nVideoWidth ||
		nHeight != m_nVideoHeight) &&
		(0 != nWidth && 0 != nHeight))
	{
		deinit();
	}

	m_csDecInit.lock();
	if (!m_bInit)
	{
		if (nWidth == 0 || nHeight == 0)
		{
			m_csDecInit.unlock();
			return 1;
		}

		init(nWidth,nHeight);
	}

	int	nResult;
	HiH264DecFrame	DecFrames;
	nResult = HiH264Dec_dll.DecFrame(m_hDec,(unsigned char *)pData,nDataLength,0,&DecFrames,0);
	if (-1 == nResult)
	{
		m_csDecInit.unlock();
		qDebug()<<"HI H264DEC need more bits";
		return 0;
	}
	else if (-3 == nResult)                 
	{
		m_csDecInit.unlock();
		return 1;
	}
	while (0 == nResult)
	{
		QVariantMap decodeParam;

		decodeParam.insert("data",(int)DecFrames.pY);
		decodeParam.insert("Ydata",(int)DecFrames.pY);
		decodeParam.insert("Udata",(int)DecFrames.pU);
		decodeParam.insert("Vdata",(int)DecFrames.pV);
		decodeParam.insert("width",(int)(DecFrames.uWidth - DecFrames.uCroppingLeftOffset - DecFrames.uCroppingRightOffset));
		decodeParam.insert("height",(int)(DecFrames.uHeight - DecFrames.uCroppingTopOffset - DecFrames.uCroppingBottomOffset));
		decodeParam.insert("YStride",(int)DecFrames.uYStride);
		decodeParam.insert("UVStride",(int)DecFrames.uUVStride);
		decodeParam.insert("lineStride",0);
		decodeParam.insert("pixelFormat",0);
		decodeParam.insert("flags",0);

		//äÖÈ¾
		g_csDecInit.lock();
		int ret = applyEventProc("DecodedFrame",decodeParam);
		g_csDecInit.unlock();

		m_nVideoWidth = DecFrames.uWidth;
		m_nVideoHeight = DecFrames.uHeight;

		nResult = HiH264Dec_dll.DecFrame(m_hDec,NULL,0,0,&DecFrames,0);
		if (-1 == nResult)
		{
			break;
		}

	}
	m_csDecInit.unlock();
	return 0;

}
int Hih264Decoder::flushDecoderBuffer()
{
	m_csDecInit.lock();
	if (!m_bInit)
	{
		m_csDecInit.unlock();
		return 1;
	}
	m_csDecInit.unlock();
	return 0;
}

QStringList Hih264Decoder::eventList()
{
	return m_eventMap.keys();
}
int Hih264Decoder::queryEvent(QString eventName,QStringList& eventParams)
{
	eventParams.clear();
	eventParams<<"data"<<"Ydata"<<"Udata"<<"Vdata"<<"width"<<"height"
		<<"YStride"<<"UVStride"<<"lineStride"<<"pixelFormat"<<"flags";

	return true;
}
int Hih264Decoder::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	EventData evdata={proc,pUser};
	if (m_eventMap.find(eventName)==m_eventMap.end())

	{
		m_eventMap.insert(eventName,evdata);
		return true;
	}
	else return false;
}

int Hih264Decoder::applyEventProc(QString eventName,QVariantMap searchinfo)
{
	QMap<QString,EventData>::Iterator eviter = m_eventMap.find(eventName);
	if(eviter!=m_eventMap.end())
	{
		eventcallback evcallback=eviter->eventproc;
		if (evcallback)
		{
			evcallback(eventName,searchinfo,eviter->puser);
			return true;
		}
	}

	return false;
}

QString Hih264Decoder::getModeName()
{
	return QString("VideoDecoder");
}

long __stdcall Hih264Decoder::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IVideoDecoder == iid)
	{
		*ppv = static_cast<IVideoDecoder *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall Hih264Decoder::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall Hih264Decoder::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}