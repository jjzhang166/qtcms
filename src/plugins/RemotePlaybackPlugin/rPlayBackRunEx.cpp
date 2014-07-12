#include "rPlayBackRunEx.h"


rPlayBackRunEx::rPlayBackRunEx(void)
{
	m_sVendorList<<"DVR"<<"IPC"<<"NVR";
}


rPlayBackRunEx::~rPlayBackRunEx(void)
{
}

void rPlayBackRunEx::run()
{

}

int rPlayBackRunEx::setDeviceHostInfo( const QString &sAddress,unsigned int uiPort,const QString &sEsee )
{
	if (!QThread::isRunning())
	{
		if (!m_tRecDeviceInfo.tAddress.setAddress(sAddress)||uiPort>65535)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as the input param error";
			return 1;
		}else{
			m_tRecDeviceInfo.sAddress=sAddress;
			m_tRecDeviceInfo.uiPort=uiPort;
			m_tRecDeviceInfo.sEsee=sEsee;
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::setDevcieVendor( const QString& sVendor )
{
	if (!QThread::isRunning())
	{
		if (sVendor.isEmpty()||!m_sVendorList.contains(sVendor))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDevcieVendor fail as the input sVendor is empty or undefined";
			return 1;
		}else{
			m_tRecDeviceInfo.sVendor=sVendor;
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDevcieVendor fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::setUserVerifyInfo( const QString &sUserName,const QString &sPassword )
{
	if (!QThread::isRunning())
	{
		m_tRecDeviceInfo.sUserName=sUserName;
		m_tRecDeviceInfo.sPassword=sPassword;
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setUserVerifyInfo fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::addChannelIntoPlayGroup( unsigned int uiWin,int nChannel )
{
	if (!QThread::isRunning())
	{
		tagWinChannelInfo tWinChannelInfo;
		tWinChannelInfo.uiWin=uiWin;
		tWinChannelInfo.nChannel=nChannel;
		m_tWinChannelInfo.insert(uiWin,tWinChannelInfo);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"addChannelIntoPlayGroup fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::startSearchRecFile( int nChannel,int nTypes,const QString &sStartTime,const QString &sEndTime )
{
	if (!QThread::isRunning())
	{
		if (nTypes<0||nTypes>15||sStartTime>=sEndTime)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile as the input params are error";
			return 2;
		}else{
			m_tRecDeviceInfo.sSearchStartTime=sStartTime;
			m_tRecDeviceInfo.sSearchEndTime=sEndTime;
			m_tRecDeviceInfo.nSearchChannel=nChannel;
			m_tRecDeviceInfo.nSearchTypes=nTypes;
			m_qStepCode.clear();
			m_qStepCode.enqueue(RECSEARCHFILE);
			QThread::start();
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as work thread had been running ,if you want to reset the param please wait";
		return 1;
	}
}

int rPlayBackRunEx::groupPlay( int nTypes,const QString &sStartTime,const QString &sEndTime )
{
	if (!QThread::isRunning())
	{
		if (nTypes<0||nTypes>15||sStartTime>=sEndTime)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as the input param is error";
			return 2;
		}else{
			m_tRecDeviceInfo.nPlayTypes=nTypes;
			m_tRecDeviceInfo.sPlayStartTime=sStartTime;
			m_tRecDeviceInfo.sPlayEndTime=sEndTime;
			m_qStepCode.clear();
			m_qStepCode.enqueue(RECGROUPPLAY);
			QThread::start();
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as work thread had been running,if you want replay please call groupStop()";
		return 1;
	}
	return 0;
}

int rPlayBackRunEx::groupPause()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPPAUSE);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPause fail as the work thread is not running";
		return 1;
	}
}

int rPlayBackRunEx::groupContinue()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPCONTINUE);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupContinue fail as the work thread is not running";
		return 1;
	}
}

int rPlayBackRunEx::groupStop()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPSTOP);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay had been running ,there is no need to call groupStop again";
		return 1;
	}
}

int rPlayBackRunEx::groupSpeedFast()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPSPEEDFAST);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupSpeedFast fail as groupPlay is no in running";
		return 1;
	}
}

int rPlayBackRunEx::groupSpeedSlow()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPSEEEDSLOW);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupSpeedSlow fail as groupPlay is no in running";
		return 1;
	}
}

int rPlayBackRunEx::groupSpeedNormal()
{
	if (QThread::isRunning())
	{
		m_qStepCode.enqueue(RECGROUPNORMAL);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupSpeedNormal fail as groupPlay is no in running";
		return 1;
	}
}

int rPlayBackRunEx::setVolume( const unsigned int &uiPersent ,QWidget *pWin)
{
	if (QThread::isRunning())
	{
		m_tRecDeviceInfo.uiPersent=uiPersent;
		m_tRecDeviceInfo.pWin=pWin;
		m_qStepCode.enqueue(RECSETVOLUME);
		return 0;
	}else{
		//do nothing
		return 1;
	}
}

int rPlayBackRunEx::audioEnable( bool bEnable )
{
	if (QThread::isRunning())
	{
		m_tRecDeviceInfo.bAudioEnable=bEnable;
		m_qStepCode.enqueue(RECAUDIOENABLE);
		return 0;
	}else{
		//do nothing
		return 1;
	}
}

QString rPlayBackRunEx::getNowPlayTime()
{
	return "";
}

int rPlayBackRunEx::cbRecRunFoundFile( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunFileSearchFinished( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunFileSearchFail( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunSocketError( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunStateChange( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

int rPlayBackRunEx::cbRecRunCacheState( QString evName,QVariantMap evMap,void*pUser )
{
	return 0;
}

void rPlayBackRunEx::registerEvent( QString sEventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser )
{
	if (!m_tEventNameList.contains(sEventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event :"<<sEventName<<"fail";
		return;
	}else{
		tagRecPlayBackProcInfo tProcInfo;
		tProcInfo.proc=proc;
		tProcInfo.pUser=pUser;
		m_tEventMap.insert(sEventName,tProcInfo);
		return;
	}
}

void rPlayBackRunEx::eventCallBack( QString sEventName,QVariantMap evMap )
{
	if (m_tEventNameList.contains(sEventName))
	{
		tagRecPlayBackProcInfo tProcInfo=m_tEventMap.value(sEventName);
		if (NULL!=tProcInfo.proc)
		{
			tProcInfo.proc(sEventName,evMap,tProcInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<"event is not regist";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support event :"<<sEventName;
	}
}

int cbXRecRunFoundFile( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunFoundFile(evName,evMap,pUser);
}

int cbXRecRunFileSearchFinished( QString evName,QVariantMap evMap,void*pUser )
{

	return ((rPlayBackRunEx*)pUser)->cbRecRunFileSearchFinished(evName,evMap,pUser);
}

int cbXRecRunFileSearchFail( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunFileSearchFail(evName,evMap,pUser);
}

int cbXRecRunSocketError( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunSocketError(evName,evMap,pUser);
}

int cbXRecRunStateChange( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunStateChange(evName,evMap,pUser);
}

int cbXRecRunCacheState( QString evName,QVariantMap evMap,void*pUser )
{
	return ((rPlayBackRunEx*)pUser)->cbRecRunCacheState(evName,evMap,pUser);
}
