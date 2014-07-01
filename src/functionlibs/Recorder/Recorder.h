#ifndef RECORDER_H
#define RECORDER_H

#include "Recorder_global.h"
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QDebug>
#include "IRecorder.h"
#include "StorageMgr.h"
#include <IEventRegister.h>
#include <QTimer>

//#define REC_SYS_DATA					0x11
#define AVENC_IDR		0x01
#define AVENC_PSLICE	0x02
#define AVENC_AUDIO		0x00

class Recorder : public QThread,
	public IRecorder,
	public IEventRegister
{
	Q_OBJECT
public:
	Recorder();
	~Recorder();

	//IRecorder
	virtual int Start();
	virtual int Stop();

	virtual int InputFrame(QVariantMap& frameinfo);

	virtual int SetDevInfo(const QString& devname,int nChannelNum);

	virtual QString getModeName();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);


	typedef struct _tagVideoInfo{
		unsigned int uiWidth;
		unsigned int uiHeight;
	}VideoInfo;

	typedef struct _tagRecNode{
		int    nChannel;
		unsigned int  dwDataType;
		unsigned int  dwBufferSize;
		unsigned int  dwTicketCount;

		char * Buffer;
		int samplerate;
		int samplewidth;
		char encode[8];
	}RecBufferNode;

public slots:
	void checkdiskfreesize();
protected:
	void run();
private:
	bool CreateSavePath(QString& sSavePath, QTime &start);
	bool CreateDir(QString fullname);
	void cleardata();
	void enventProcCall(QString sEvent,QVariantMap parm);
	unsigned int getSeconds(QString &fileName);
	QString getFileEndTime(QString fileName, QTime start);
	qint64 getFileSize(QString fileName);

	int m_nRef;
	QMutex m_csRef;

	//
	QString m_devname;
	int m_channelnum;
	//
	int m_nRecWidth;
	int m_nRecHeight;

	unsigned int m_nFrameCount;
	unsigned int m_nLastTicket;
	unsigned int m_nFrameCountArray[31];

	//bool m_stop;
	bool m_bFinish;
	int m_filesize;
	int m_reservedsize;
	QMutex m_dataRef;
	QQueue<RecBufferNode> m_dataqueue;
	StorageMgr m_StorageMgr;
	//proc
	QStringList m_eventList;
	QMultiMap<QString, ProcInfoItem> m_eventMap;

	//checkdiskfreesize
	bool m_bcheckdiskfreesize;
	QTimer m_checkdisksize;
};

#endif // RECORDER_H
