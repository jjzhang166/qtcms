#pragma once
#include <QThread>
#include <IEventRegister.h>
#include <IRecorder.h>
#include "IRecorderEx.h"
#include <QMutex>
#include <QMultiMap>
#include <QDebug>
#include <QQueue>
#include <QEventLoop>
#include <QTimer>
#include <avilib.h>
#include <Recorder_global.h>
#include "RepairDatabase.h"

#define AVENC_IDR		0x01
#define AVENC_PSLICE	0x02
#define AVENC_AUDIO		0x00
#define DATA_QUEUE_MAX_SIZE 240 //最大存储5分钟的缓存，如果超过，就会丢弃
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
	QString sApplyDisk;//e:
	QString sFilePath;
	QString sDeviceName;
	QString sStartTime;
	QString sEndTime;
	QString sStartDate;
	int iChannel;
	int iFileSize;
	int uiRecorderId;
	int uiSearchId;
	int iWindId;
	int iRecordType;
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
	public IRecorderEx,
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

	//IRecordEx
	virtual int SetDevInfoEx(const int &nWindId, const int &nRecordType);
	virtual int FixExceptionalData();

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
	int checkALL(avi_t *pAviFile);//0:接着录像；1：录像打包；2：产生错误，停止录像
	int checkFileSize(avi_t *pAviFile);//0:文件大小足够了，可以打包；1：表示文件大小还不足，接着录像，2：表示有错误
	int checkDiskSize();//0:表示磁盘空间足够，接着录像；1：表示磁盘空间不足
	bool upDateDataBase(QString sEndTime);//更新数据库，0：表示更新成功；1：表示更新失败
	bool packFile(avi_t *pAviFile);
	bool applyDiskSpace();//申请空间
	bool createRecordItem();//创建录像数据条目
	bool createSearchItem();//创建搜索表条目
	bool createFileDir();
	bool resetCurrentRecordId();
	bool updateSearchDataBase(QString sEndTime);
	bool deleteSearchDataBaseItem();
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
	volatile bool m_bFull;
	tagRecorderExInfo m_tRecorderInfo;
	quint64 m_iFristPts;
	quint64 m_iLastPts;
	int m_iCheckBlockCount;
	int m_iChecKFileSizeCount;
	int m_iCheckDiskSizeCount;
	int m_iUpdateDatabaseCount;
	bool m_bChecKFileSize;
	bool m_bCheckDiskSize;
	bool m_bUpdateDatabase;
	bool m_bIsCreateSearchItem;
};

