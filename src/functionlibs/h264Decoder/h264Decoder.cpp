#include "h264Decoder.h"
#include "h264wh.h"
#include <guid.h>

typedef struct _tagFrameData{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int nWidth;
	int nHeight;
	int nYStride;
	int nUVStride;
}FrameData;

QMutex g_csInitAvlib;
h264Decoder::h264Decoder() :
m_nRef(0),
m_outbuf(NULL),
m_nVideoHeight(0),
m_nVideoWidth(0),
m_hDec(0)
{

}

h264Decoder::~h264Decoder()
{

}

int h264Decoder::init(int nWidth,int nHeight)
{
	m_hDec = avlib_dll.CreateVideoDecoder();
	if (0 == m_hDec)
	{
		return false;
	}
	int ret = avlib_dll.VideoDecoderInit(m_hDec,AV_CODEC_ID_H264,nWidth,nHeight);
	if (0 != ret)
	{
		return false;
	}
	if (m_outbuf != NULL)
	{
		delete m_outbuf;
		m_outbuf = NULL;
	}

	m_outbuf = new unsigned char[nWidth*nHeight*2];
	m_nVideoWidth = nWidth;
	m_nVideoHeight = nHeight;
	m_bInit = true;

	return true;
}
int h264Decoder::deinit()
{
	if (0 != avlib_dll.VideoDeocderRelease(m_hDec))
	{
		m_csDecInit.unlock();
	}
	m_hDec = 0;
	if (m_outbuf != NULL)
	{
		delete m_outbuf;
		m_outbuf = NULL;
	}

	m_nVideoWidth = 0;
	m_nVideoHeight = 0;

	m_bInit = false;

	return true;
}
int h264Decoder::decode(char * pData,unsigned int nDataLength)
{
	m_csDecInit.lock();
	int nWidth = 0;
	int nHeight = 0;
	GetWidthHeight((char *)pData,nDataLength,&nWidth,&nHeight);

	if ((nWidth != m_nVideoWidth ||
		nHeight != m_nVideoHeight) &&
		(0 != nWidth && 0 != nHeight))
	{
		g_csInitAvlib.lock();
		deinit();
		g_csInitAvlib.unlock();
	}

	if(!m_bInit)
	{	
		if (0 == nWidth ||
			0 == nHeight)
		{
			m_csDecInit.unlock();
		}
		g_csInitAvlib.lock();
		init(nWidth,nHeight);
		g_csInitAvlib.unlock();
	}

	int outsize = 0;
	FrameData temp;	
	memset(&temp,0,sizeof(FrameData));

	QVariantMap decodeParam;

	g_csInitAvlib.lock();
	avlib_dll.VideoDecoderDecode(m_hDec,(unsigned char*)pData,nDataLength,m_outbuf,&outsize,0,0,&temp);
	g_csInitAvlib.unlock();
	m_csDecInit.unlock();

	decodeParam.insert("data",(int)temp.pY);
	decodeParam.insert("Ydata",(int)temp.pY);
	decodeParam.insert("Udata",(int)temp.pU);
	decodeParam.insert("Vdata",(int)temp.pV);
	decodeParam.insert("width",(int)temp.nWidth);
	decodeParam.insert("height",(int)temp.nHeight);
	decodeParam.insert("YStride",(int)temp.nYStride);
	decodeParam.insert("UVStride",(int)temp.nUVStride);
	decodeParam.insert("lineStride",0);
	decodeParam.insert("pixelFormat",0);
	decodeParam.insert("flags",0);

	bool ret = false;
	qDebug("%d\n%d\n%d\n",temp.pY,temp.pU,temp.pV);
	if (outsize > 0)
	{
		//‰÷»æ
		g_csInitAvlib.lock();
		ret=applyEventProc("DecodedFrame",decodeParam);
		g_csInitAvlib.unlock();
	}

	return ret;
}
int h264Decoder::flushDecoderBuffer()
{
	return false;
}

QStringList h264Decoder::eventList()
{
	return m_eventMap.keys();
}
int h264Decoder::queryEvent(QString eventName,QStringList& eventParams)
{
	eventParams.clear();
	eventParams<<"data"<<"Ydata"<<"Udata"<<"Vdata"<<"width"<<"height"
		<<"YStride"<<"UVStride"<<"lineStride"<<"pixelFormat"<<"flags";

	return true;
}
int h264Decoder::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	EventData evdata={proc,pUser};
	if (m_eventMap.find(eventName)==m_eventMap.end())

	{
		m_eventMap.insert(eventName,evdata);
		return true;
	}
	else return false;
}

int h264Decoder::applyEventProc(QString eventName,QVariantMap searchinfo)
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

QString h264Decoder::getModeName()
{
	return QString("VideoDecoder");
}

long __stdcall h264Decoder::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall h264Decoder::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall h264Decoder::Release()
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