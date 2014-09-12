#include "recorddat.h"

recordDatCore *g_pRecordDatCore=NULL;
uint g_nRecordDatCoreCount=0;
QMutex g_tRecordDatCoreLock;
void initRecordDatCore(recordDatCore **pRecordDatCore){
	g_tRecordDatCoreLock.lock();
	if (NULL==g_pRecordDatCore)
	{
		g_pRecordDatCore=new recordDatCore;
		g_nRecordDatCoreCount=0;
	}else{
		//do nothing
	}
	g_nRecordDatCoreCount++;
	*pRecordDatCore=g_pRecordDatCore;
	g_tRecordDatCoreLock.unlock();
}
void deinitRecordDatCore(){
	g_tRecordDatCoreLock.lock();
	g_nRecordDatCoreCount--;
	if (g_nRecordDatCoreCount==0)
	{
		if (NULL!=g_pRecordDatCore)
		{
			delete g_pRecordDatCore;
			g_pRecordDatCore=NULL;
		}
	}else{
		//do nothing
	}
	g_tRecordDatCoreLock.unlock();
}
#include <guid.h>
RecordDat::RecordDat():m_nRef(0),
	m_nStatus(0),
	m_nMotionTime(0),
	m_bInit(false)
{
	m_tEventList<<"RecordState"<<"RecordCore";
	recordDatCore *m_pRecordDatCore;
	initRecordDatCore(&m_pRecordDatCore);
}

RecordDat::~RecordDat()
{
	deinitRecordDatCore();
}

long __stdcall RecordDat::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister*>(this);
	}else if (IID_IRecordDat==iid)
	{
		*ppv=static_cast<IRecordDat*>(this);
	}
	else{
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

bool RecordDat::init(int nWid)
{
	m_tFuncLock.lock();
	recordDatCore *pRecordDatCore=NULL;
	initRecordDatCore(&pRecordDatCore);
	if (pRecordDatCore!=NULL)
	{
		pRecordDatCore->startRecord();
		pRecordDatCore->setBufferQueue(nWid,m_tBufferQueue);
		deinitRecordDatCore();
		pRecordDatCore=NULL;

		connect(&m_tTimeRecordTimer,SIGNAL(timeout()),this,SLOT(slCheckTimeRecord()));
		connect(&m_tMotionRecordTimer,SIGNAL(timeout()),this,SLOT(slCheckMotionRecord()));
		m_tTimeRecordTimer.start(1000);
		m_nWnd=nWid;

		m_bInit=true;
		m_tFuncLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"init fail as pRecordDatCore is null";
		abort();
		m_tFuncLock.unlock();
		return false;
	}
}

bool RecordDat::deinit()
{
	m_tFuncLock.lock();
	m_tTimeRecordTimer.stop();
	disconnect(&m_tTimeRecordTimer,SIGNAL(timeout()),this,SLOT(slCheckTimeRecord()));
	disconnect(&m_tMotionRecordTimer,SIGNAL(timeout()),this,SLOT(slCheckMotionRecord()));
	m_bInit=false;
	m_tBufferQueue.clear();
	setRecordType(MOTIONRECORD,false);
	setRecordType(TIMERECORD,false);
	setRecordType(MANUALRECORD,false);
	recordDatCore *pRecordDatCore=NULL;
	initRecordDatCore(&pRecordDatCore);
	if (pRecordDatCore!=NULL)
	{
		pRecordDatCore->removeBufferQueue(m_nWnd);
		deinitRecordDatCore();
		pRecordDatCore=NULL;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"there may be some error,please check";
	}
	m_tFuncLock.unlock();
	return true;
}

int RecordDat::inputFrame( QVariantMap &tFrameInfo )
{
	if (m_bInit)
	{
		if (m_nWnd<64&&m_nWnd>=0)
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"m_nWnd is illegal :"<<m_nWnd;
			abort();
		}
		tFrameInfo.insert("winid",m_nWnd);
		m_tBufferQueue.enqueue(tFrameInfo);
	}else{
		//do nothing
	}
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
		if (checkMotionRecordSchedule())
		{
			if (setRecordType(MOTIONRECORD,true))
			{
				m_tMotionRecordTimer.stop();
				m_tMotionRecordTimer.start(nTime);
				m_tFuncLock.unlock();
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"motionRecordStart fail as setRecordType fail";
			}
		}else{
			//do nothing
			qDebug()<<__FUNCTION__<<__LINE__<<"motionRecordStart fail as current time is not in MotionRecordSchedule";
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
	int nTotal=MANUALRECORD+TIMERECORD+MOTIONRECORD;
	recordDatCore *pRecordDatCore=NULL;
	initRecordDatCore(&pRecordDatCore);
	if (pRecordDatCore!=NULL)
	{
		bSetFlags=pRecordDatCore->setRecordType(m_nWnd,nType,bFlags);
		deinitRecordDatCore();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setRecordType fail as pRecordDatCore should not been null";
		m_tSetRecordTypeLock.unlock();
		abort();
		return false;
	}
	if (nType==MANUALRECORD||nType==TIMERECORD||nType==MOTIONRECORD)
	{
		if (bFlags)
		{
			m_nStatus=nType|m_nStatus;
		}else{
			m_nStatus=(nTotal-nType)&m_nStatus;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setRecordType fail as nType is undefined"<<nType;
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
	int nCurrentWeekDay=QDate::currentDate().dayOfWeek()-1;
	QTime tCurrentTime=QTime::currentTime();
	bool bRecordFlags=false;
	m_tRecordDatTimeListLock.lock();
	for(int i=0;i<m_tRecordDatTimeList.size();i++){
		tagRecordDatTimeInfo tRecTimeInfo=m_tRecordDatTimeList.at(i);
		if (tRecTimeInfo.nWeekDay==nCurrentWeekDay)
		{
			if (tRecTimeInfo.tStartTime<=tCurrentTime&&tRecTimeInfo.tEndTime>tCurrentTime)
			{
				bRecordFlags=true;
				break;
			}
		}else{
			//do nothing
		}
	}
	m_tRecordDatTimeListLock.unlock();
	if (bRecordFlags)
	{
		//¿ªÆô¶¨Ê±Â¼Ïñ
		if (setRecordType(TIMERECORD,true))
		{
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"start time record fail as setRecordType fail";
		}
	}else{
		//Í£Ö¹¶¨Ê±Â¼Ïñ
		if (setRecordType(TIMERECORD,false))
		{

		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"stop time record fail as setRecordType fail";
		}
	}
}

bool RecordDat::updateRecordSchedule(int nChannelId )
{
	ISetRecordTime *pSetRecordTime=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_ISetRecordTime,(void**)&pSetRecordTime);
	if (NULL!=pSetRecordTime)
	{
		QStringList tRecordIdList=pSetRecordTime->GetRecordTimeBydevId(nChannelId);
		tagRecordDatTimeInfo tRecTimeInfo;
		m_tRecordDatTimeListLock.lock();
		m_tRecordDatTimeList.clear();
		for (int i=0;i<tRecordIdList.size();i++)
		{
			QString sRecordId=tRecordIdList[i];
			QVariantMap tTimeInfo=pSetRecordTime->GetRecordTimeInfo(sRecordId.toInt());
			tRecTimeInfo.nEnable=tTimeInfo.value("enable").toInt();
			tRecTimeInfo.nWeekDay=tTimeInfo.value("weekday").toInt();
			tRecTimeInfo.tStartTime=QTime::fromString(tTimeInfo.value("starttime").toString().mid(11),"hh:mm:ss");
			tRecTimeInfo.tEndTime=QTime::fromString(tTimeInfo.value("endtime").toString().mid(11),"hh:mm:ss");
			if (tRecTimeInfo.nEnable==1)
			{
				m_tRecordDatTimeList.append(tRecTimeInfo);
			}else{
				//do nothing
			}
		}
		m_tRecordDatTimeListLock.unlock();
		pSetRecordTime->Release();
		pSetRecordTime=NULL;
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"updateRecordSchedule fail as pSetRecordTime is null";
	}
	return false;
}

void RecordDat::slCheckMotionRecord()
{
	if (m_nStatus&MOTIONRECORD)
	{
		if (setRecordType(MOTIONRECORD,false))
		{
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"stop motion record fail";
		}
	}else{
		//do nothing
	}
}

bool RecordDat::checkMotionRecordSchedule()
{
	int nCurrentWeekDay=QDate::currentDate().dayOfWeek()-1;
	QTime tCurrentTime=QTime::currentTime();
	bool bRecordFlags=false;
	m_tRecordDatTimeListLock.lock();
	for(int i=0;i<m_tRecordDatTimeList.size();i++){
		tagRecordDatTimeInfo tRecTimeInfo=m_tRecordDatTimeList.at(i);
		if (tRecTimeInfo.nWeekDay==nCurrentWeekDay)
		{
			if (tRecTimeInfo.tStartTime<=tCurrentTime&&tRecTimeInfo.tEndTime>tCurrentTime)
			{
				bRecordFlags=true;
				break;
			}
		}else{
			//do nothing
		}
	}
	m_tRecordDatTimeListLock.unlock();
	if (bRecordFlags)
	{
		return true;
	}else{
		return false;
	}
}

bool RecordDat::upDateSystemDatabase()
{
	recordDatCore *pRecordDatCore=NULL;
	initRecordDatCore(&pRecordDatCore);
	if (pRecordDatCore!=NULL)
	{
		pRecordDatCore->reloadSystemDatabase();
		deinitRecordDatCore();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"upDateSystemDatabase fail as pRecordDatCore is null";
		abort();
		return false;
	}
}
