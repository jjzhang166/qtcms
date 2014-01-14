#pragma once
#include <QtCore/QThread>
#include <QtCore/QQueue>
#include <QtCore/QMutex>
#include <QtCore/QDateTime>
#include <QtCore/QVariantMap>
#include "IDeviceConnection.h"
#include "IRemotePlayback.h"
#include "IEventRegister.h"
#include "avilib.h"

class RemoteBackup : public QThread
{
public:
	RemoteBackup(void);
	~RemoteBackup(void);

	typedef struct _tagRecFrame {
		int type;
		char* pdata;
		unsigned int datasize;

		unsigned int pts;
		unsigned int gentime;
	}RecFrame;
public:
	int StartByParam(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
			int nChannel,
			int nTypes,
			const QDateTime & startTime,
			const QDateTime & endTime,
			const QString & sbkpath);
	int Stop();

	float getProgress();

	int WriteFrameData(QVariantMap &frameinfo);

	int SetBackupEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
protected:
	void run();
private:
	typedef int (__cdecl *BackProc)(QString,QVariantMap,void *);
	typedef struct _evItem{
		BackProc backproc;
		void* pUser;
	}evItem;
	//bool checkParam(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,int nChannel);
	bool connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
	bool tryConnectProtocol(CLSID clsid,const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
	void stopConnect();
	void callBackupStatus(QString sstatus);
	/*bool createFile(const QString &sPath,int nchannel,QDateTime startTime);*/
	bool createFile();
	int closeFile();
	void clearbuffer();

	QMutex m_bufflock;
	QQueue<RecFrame> m_bufferqueue;
	IDeviceConnection* m_pBackupConnect;
	IRemotePlayback* m_pRemotePlayback;
	QString m_filePath;
	//QString m_fullfilename;
	evItem m_backproc;
	
	avi_t * AviFile;
	int m_videoHeight;
	int m_videoWidth;
	int m_samplerate;
	int m_samplewidth;

	bool m_bAudioBeSet;
	bool m_backuping;
	bool m_bFinish;

	unsigned int m_nFrameCount;
	unsigned int m_nLastTicket;
	unsigned int m_nFrameCountArray[31];

	/*unsigned int m_sttime;
	unsigned int m_edtime;*/
	
	float m_progress;
	unsigned int m_firstgentime;
	
	QDateTime m_stime;
	QDateTime m_etime;
	int m_nchannel;
	QString m_savePath;
	QString m_devid;
};

