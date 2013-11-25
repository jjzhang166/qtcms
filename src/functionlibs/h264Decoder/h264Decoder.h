#ifndef H264DECODER_H
#define H264DECODER_H

#include "h264Decoder_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include "IVideoDecoder.h"
#include "IEventRegister.h"
#include "AvLibDll.h"

//#define EV_SEARCHINFO "DecodedFrame"
typedef int (__cdecl *eventcallback)(QString,QVariantMap,void *);

class h264Decoder :
	public IVideoDecoder,
	public IEventRegister
{
public:
	h264Decoder();
	~h264Decoder();

	virtual int init(int nWidth,int nHeight);
	virtual int deinit();
	virtual int decode(char * pData,unsigned int nDataLength);
	virtual int flushDecoderBuffer();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	virtual QString getModeName();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();
private:
	int applyEventProc(QString eventName,QVariantMap searchinfo);

	typedef struct __EventData{
		eventcallback eventproc;
		void* puser;
	}EventData;
	QMap<QString,EventData> m_eventMap;

	int m_nRef;
	QMutex m_csRef;

	CAvLibDll avlib_dll;
	int m_hDec;
	int m_nVideoWidth;
	int m_nVideoHeight;
	bool m_bInit;
	unsigned char* m_outbuf;
	QMutex m_csDecInit;
};

#endif // H264DECODER_H
