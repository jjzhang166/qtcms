#pragma once
#include <QThread>
#include <IEventRegister.h>
#include <IRecorder.h>
#include <QMutex>
#include <QMultiMap>
#include <QDebug>
#include <QQueue>
#include <QEventLoop>
#include <QTimer>
#include <avilib.h>

#define AVENC_IDR		0x01
#define AVENC_PSLICE	0x02
#define AVENC_AUDIO		0x00
#define DATA_QUEUE_MAX_SIZE 120 //最大存储5分钟的缓存，如果超过，就会丢弃
typedef int (__cdecl *recorderExEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagRecorderExProcInfo{
	recorderExEventCb proc;
	void *pUser;
}tagRecorderExProcInfo;
typedef struct __tagRecorderExNode{
	int iChannel;
	unsigned int uiDataType;
	unsigned int uiBufferSize;
	unsigned int uiTicketCount;
	char *pBuffer;
	int iSampleRate;
	int iSampleWidth;
	char cEncode[8];
}tagRecorderExNode;
typedef struct __tagRecorderExInfo{
	QString sFilePath;
	QString sDeviceName;
	int iChannel;
}tagRecorderExInfo;
typedef enum __tagRecorderStepCode{
	REC_INIT,//新文件的各项参数初始化
	REC_FRIST_I_FRAME,//等待第一个I帧
	REC_CREATE_PATH,//创建文件路径：创建录像数据表，录像搜索表，申请空间，创建文件夹
	REC_OPEN_FILE,//打开文件
	REC_SET_VIDEO_PARM,// 设置文件（视频）的各项参数
	REC_SET_AUDIO_PARM,//设置文件（音频）的各项参数
	REC_WRITE_FRAME,//写文件
	REC_CHECK_AND_UPDATE,//检测硬盘空间,检测文件大小,更新数据库
	REC_WAIT_FOR_PACK,
	REC_PACK,//文件打包
	REC_ERROR,
	REC_END,
}tagRecorderStepCode;
class RecorderEx:public QThread,
	public IRecorder,
	public IEventRegister
{
	Q_OBJECT
public:
	RecorderEx(void);
	~RecorderEx(void);
public:
	//IRecorder
	virtual int Start();
	virtual int Stop();
	virtual int InputFrame(QVariantMap& frameinfo);
	virtual int SetDevInfo(const QString& devname,int nChannelNum);

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
protected:
	void run();
private slots:
	void slCheckBlock();
private:
	void clearData();
	void sleepEx(int iTime);
	void eventProcCall(QString sEvent,QVariantMap tInfo);
	bool createFilePath();
	int checkALL();//0:接着录像；1：录像打包；2：产生错误，停止录像
	int checkFileSize();//0:文件大小足够了，可以打包；1：表示文件大小还不足，接着录像，2：表示有错误
	int checkDiskSize();//0:表示磁盘空间足够，接着录像；1：表示磁盘空间不足
	bool upDateDataBase();//更新数据库，0：表示更新成功；1：表示更新失败
	bool packFile(avi_t *pAviFile);
private:
	int m_nRef;
	QMutex m_csRef;
	QMutex m_csDataLock;
	QStringList m_tEventList;
	QMultiMap<QString,tagRecorderExProcInfo> m_tEventMap;
	bool m_bStop;
	QQueue<tagRecorderExNode> m_tDataQueue;
	int m_iSleepSwitch;
	QTimer m_tCheckBlockTimer;
	bool m_bBlock;
	int m_iPosition;
	int m_iVideoWidth;
	int m_iVideoHeight;
	int m_iFrameCount;
	int m_iLastTicket;
	unsigned int m_uiFrameCountArray[31];
	bool m_bFull;
	tagRecorderExInfo m_tRecorderInfo;
	quint64 m_iFristPts;
	quint64 m_iLastPts;
	int m_iCheckBlockCount;
	bool m_bChecKFileSize;
	bool m_bCheckDiskSize;
	bool m_bUpdateDatabase;
};

