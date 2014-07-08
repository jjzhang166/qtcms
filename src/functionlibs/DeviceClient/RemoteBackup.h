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
#include <QTextCodec>
#include <IDisksSetting.h>

#include <QDebug>
#include <QStringList>
#include <QMultiMap>
#include <QTimer>
typedef struct __tagBackUpInfo{
	QString sAddr;
	unsigned int uiPort;
	QString sEeeId;
	int nChannel;
	int nTypes;
	QDateTime startTime;
	QDateTime endTime;
	QString sPath;

}tagBackUpInfo;
typedef enum __tagStep{
	INIT,//
	CONNECTTODEVICE,//
	INITPACK,
	FIRST_I_FRAME,
	OPEN_FILE,
	SET_VIDEO,
	WRITE_FRAME,
	CHECK_FILE_SIZE,
	CHECK_DISK_SPACK,
	WAIT_FOR_PACK,
	PACK,
	FAIL,
	SUCCESS,
	END,

}tagStep;
class RemoteBackup : public QThread
{
	Q_OBJECT
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
private slots:
	void slCheckDisk();
	void slCheckBlock();
private:
	typedef int (__cdecl *BackProc)(QString,QVariantMap,void *);
	typedef struct _evItem{
		BackProc backproc;
		void* pUser;
	}evItem;
	bool connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
	bool tryConnectProtocol(CLSID clsid,const QString &sAddr,unsigned int uiPort,const QString &sEseeId);

	void eventProcCall(QString sEventName,QVariantMap parm);

	void clearbuffer();
	bool getUsableDisk(QString disk);

	QMutex m_bufflock;
	QQueue<RecFrame> m_bufferqueue;
	IDeviceConnection* m_pBackupConnect;

	evItem m_backproc;

	int m_videoHeight;
	int m_videoWidth;
	int m_samplerate;
	int m_samplewidth;

	bool m_bAudioBeSet;
	bool m_backuping;
	bool m_bCheckDisk;
	bool m_bCheckBlock;

	int m_nPosition;

	unsigned int m_nFrameCount;
	unsigned int m_nLastTicket;
	unsigned int m_nFrameCountArray[31];


	float m_progress;
	unsigned int m_firstgentime;
	

	tagBackUpInfo m_tBackUpInfo;

	QStringList m_eventList;
	QMultiMap<QString,evItem> m_eventMap;
	QTimer m_tCheckDisk;
	QTimer m_tCheckBlock;
};

