#pragma once
#include <QThread>
#include <guid.h>
#include <QtNetwork/QHostAddress>
#include <IDeviceRemotePlayback.h>
#include <QDateTime>
#include <IDeviceSearchRecord.h>
#include <IDeviceClient.h>
class RemotePlaybackThread:public QThread
{
	Q_OBJECT
public:
	RemotePlaybackThread(void);
	~RemotePlaybackThread(void);

public:
	int SetParm(QString m_nsUserName,QString m_nsUserPwd,uint m_nuiPort,QString m_nHostAddress,QString m_nsEseeId);
	int SetIDeviceGroupRemotePlaybackParm(IDeviceGroupRemotePlayback *lpIDeviceGroupRemotePlayback);
private:
	QString m_sUserName;
	QString m_sUserPwd;
	QString m_sEseeId;
	uint m_uiPort;
	QString m_HostAddress;
	IDeviceGroupRemotePlayback *LpIDeviceGroupRemotePlayback;
public slots:
	void startSearchRecFileSlots(int nChannel,int nTypes,const QString & startTime,const QString & endTime);
	void GroupPlaySlots(int nTypes,const QString & startTime,const QString & endTime);
signals:
	void finishSearchRecFileSig();
};

