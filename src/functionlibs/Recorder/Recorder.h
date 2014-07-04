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
typedef enum __tagRecStepCode{
	INIT,//新文件的各项参数初始化
	FRIST_I_FRAME,//等待第一个I帧
	CREATE_PATH,//申请空间 并 创建文件路径
	OPEN_FILE,//打开文件
	SET_VIDEO_PARM,// 设置文件（视频）的各项参数
	SET_AUDIO_PARM,//设置文件（音频）的各项参数
	WRITE_FRAME,//写文件
	CHECK_DISK_SPACE,//检测硬盘空间
	CHECK_FILE_SIZE,//检测文件大小
	WAIT_FOR_PACK,
	PACK,//文件打包
	ERROR,//错误
	END,
}tagRecStepCode;
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
private slots:
	void slBackToMainThread(QVariantMap evMap);
signals:
	void sgBackToMainThread(QVariantMap evMap);
public slots:
	void checkdiskfreesize();
	void checkIsBlock();
protected:
	void run();
	/*void runEx();*/
private:
	bool CreateSavePath(QString& sSavePath, QTime &start);
	bool CreateDir(QString fullname);
	void cleardata();
	void enventProcCall(QString sEvent,QVariantMap parm);
	unsigned int getSeconds(QString &fileName);
	QString getFileEndTime(QString fileName, QTime start);
	/*QString getFileEndTimeEx(QString fileName, QTime start);*/
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

	quint64  m_lnFirstPts;
	quint64  m_lnLastPts;
	//bool m_stop;
	volatile bool m_bFinish;
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
	bool m_bIsblock;
	QTimer m_checkdisksize;
	QTimer m_checkIsBlock;
	int m_nPosition;
};

#endif // RECORDER_H
