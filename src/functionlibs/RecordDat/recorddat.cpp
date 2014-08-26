#include "recorddat.h"
#include <guid.h>
RecordDat::RecordDat():m_nRef(0),
	m_nStatus(0),
	m_nMotionTime(0),
	m_bInit(false)
{
	m_tEventList<<"RecordState"<<"RecordCore";
}

RecordDat::~RecordDat()
{

}

long __stdcall RecordDat::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister*>(this);
	}else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall RecordDat::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall RecordDat::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

QStringList RecordDat::eventList()
{
	return m_tEventList;
}

int RecordDat::queryEvent( QString eventName,QStringList& eventParams )
{
	if (m_tEventList.contains(eventName))
	{
		if ("RecordState"==eventName)
		{
			eventParams<<"RecordState";
		}
		return IEventRegister::OK;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"queryEvent fail as m_tEventList do not contain ::"<<eventName;
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
}

int RecordDat::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (m_tEventList.contains(eventName))
	{
		tagRecordDatProcInfo proInfo;
		proInfo.proc=proc;
		proInfo.pUser=pUser;
		m_tEventMap.insert(eventName,proInfo);
		return IEventRegister::OK;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"registerEvent fail as m_tEventList do not contains::"<<eventName;
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
}

void RecordDat::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_tEventList.contains(sEvent))
	{
		tagRecordDatProcInfo tEventProc=m_tEventMap.value(sEvent);
		if (NULL!=tEventProc.proc)
		{
			tEventProc.proc(sEvent,tInfo,tEventProc.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"eventProcCall fail as event::"<<sEvent<<"do not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"eventProcCall fail as m_tEventList do not contains::"<<sEvent;
	}
}

bool RecordDat::init()
{

	return true;
}

bool RecordDat::deinit()
{
	return true;
}

int RecordDat::inputFrame( QVariantMap &tFrameInfo )
{
	return IRecordDat::OK;
}

bool RecordDat::manualRecordStart()
{
	m_tFuncLock.lock();
	if (m_bInit==true)
	{
		if (setRecordType(MANUALRECORD,true))
		{
			m_tFuncLock.unlock();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"manualRecordStart fail as setRecordType fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"manualRecordStart fail as m_bInit is fail";
	}
	m_tFuncLock.unlock();
	return false;
}

bool RecordDat::manualRecordStop()
{
	m_tFuncLock.lock();
	if (m_bInit==true)
	{
		if (!setRecordType(MANUALRECORD,false))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"manualRecordStop fail as setRecordType fail";
			m_tFuncLock.unlock();
			return false;
		}else{
		}
	}else{
	}
	m_tFuncLock.unlock();
	return true;
}

bool RecordDat::motionRecordStart( int nTime )
{
	m_tFuncLock.lock();
	if (m_bInit)
	{
		m_nMotionTime=nTime;
		if (setRecordType(MOTIONRECORD,true))
		{
			m_tFuncLock.unlock();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"motionRecordStart fail as setRecordType fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"motionRecordStart fail as m_bInit is fail";
	}
	m_tFuncLock.unlock();
	return false;
}

int RecordDat::getRecordStatus()
{
	return m_nStatus;
}

bool RecordDat::setRecordType( int nType,bool bFlags )
{
	//ÓÐÂ¼Ïñ¿ªÆô£¬Å×³öÊÂ¼þ
	//Í£Ö¹Â¼Ïñ£¬Å×³öÊÂ¼þ
	m_tSetRecordTypeLock.lock();
	bool bSetFlags=false;
	int nHisStatus=m_nStatus;
	if (nType==MANUALRECORD)
	{
		int nType;
		if (bFlags)
		{
			nType=2;
			m_nStatus=m_nStatus|nType;
		}else{
			nType=5;
			m_nStatus=m_nStatus&nType;
		}
	}else if(nType=MOTIONRECORD){

	}else if (nType==TIMERECORD)
	{

	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setRecordType fail as nType is undefined";
	}
	//Å×³öÍ£Ö¹Â¼Ïñ×´Ì¬
	if (m_nStatus==0&&nHisStatus!=0&&bSetFlags==true)
	{
		QVariantMap tInfo;
		tInfo.insert("RecordState",false);
		eventProcCall("RecordState",tInfo);
	}else{
		//do nothing
	}
	//Å×³ö¿ªÊ¼Â¼Ïñ×´Ì¬
	if (nHisStatus==0&&m_nStatus!=0&&bSetFlags==true)
	{
		QVariantMap tInfo;
		tInfo.insert("RecordState",true);
		eventProcCall("RecordState",tInfo);
	}
	if (bSetFlags)
	{
		m_tSetRecordTypeLock.unlock();
		return true;
	}else{
		m_tSetRecordTypeLock.unlock();
		return false;
	}
}

void RecordDat::slCheckTimeRecord()
{

}

bool RecordDat::updateRecordSchedule(int nChannelId )
{
	return true;
}
