#include "RemotePlaybackObject.h"
#include "rPlaybackWnd.h"

RemotePlaybackObject::RemotePlaybackObject(void):m_RemotePlaybackThread(NULL),rplaybackwnd(NULL)
{
	m_RemotePlaybackThread=new RemotePlaybackThread;
	m_RemotePlaybackThread->moveToThread(&m_workerThread);
	connect(this,SIGNAL(startSearchRecFileSignals(int ,int ,const QString & ,const QString & )),m_RemotePlaybackThread,SLOT(startSearchRecFileSlots(int ,int ,const QString & ,const QString & )),Qt::QueuedConnection);
	connect(this,SIGNAL(GroupPlaySignals(int ,const QString &,const QString &)),m_RemotePlaybackThread,SLOT(GroupPlaySlots( int ,const QString &,const QString & )),Qt::QueuedConnection);
	connect(&m_workerThread,SIGNAL(finished()),m_RemotePlaybackThread,SLOT(deleteLater()));
	connect(m_RemotePlaybackThread,SIGNAL(finishSearchRecFileSig()),this,SLOT(finishSearchRecFileSlots()));
	m_workerThread.start();
}


RemotePlaybackObject::~RemotePlaybackObject(void)
{
	m_workerThread.quit();
	m_workerThread.wait();
}

int RemotePlaybackObject::startSearchRecFile( int nChannel,int nTypes,const QString & startTime,const QString & endTime )
{
	emit startSearchRecFileSignals(nChannel,nTypes,startTime,endTime);
	return 0;
}

int RemotePlaybackObject::SetParm( QString m_sUserName,QString m_sUserPwd,uint m_uiPort,QString m_HostAddress,QString m_sEseeId )
{
	m_RemotePlaybackThread->SetParm(m_sUserName,m_sUserPwd,m_uiPort,m_HostAddress, m_sEseeId);
	return 0;
}

int RemotePlaybackObject::SetIDeviceGroupRemotePlaybackParm( IDeviceGroupRemotePlayback *lpIDeviceGroupRemotePlayback )
{
	m_RemotePlaybackThread->SetIDeviceGroupRemotePlaybackParm(lpIDeviceGroupRemotePlayback);
	return 0;
}

int RemotePlaybackObject::GroupPlay( int nTypes,const QString & startTime,const QString & endTime )
{
	emit GroupPlaySignals(nTypes,startTime,endTime);
	return 0;
}


void RemotePlaybackObject::finishSearchRecFileSlots()
{
	if (NULL!=rplaybackwnd)
	{
		RPlaybackWnd *wnd=(RPlaybackWnd *)rplaybackwnd;
		QVariantMap item;
		item.insert("total",0);
		wnd->RecFileSearchFinished(item);
	}
}

void RemotePlaybackObject::SetrPlaybackWnd( QWidget *wnd )
{
	rplaybackwnd=wnd;
}


