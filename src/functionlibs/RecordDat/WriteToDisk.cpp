#include "WriteToDisk.h"


WriteToDisk::WriteToDisk(void):m_pBuffer(NULL),
	m_bStop(true),
	m_bBlock(false),
	m_bWrite(false),
	m_nSleepSwitch(0),
	m_uiBufferSize(0)
{
	connect(&m_tCheckBlockTimer,SIGNAL(timeout()),this,SLOT(slCheckBlock()));	
	m_tEventList<<"sFilePath";
}


WriteToDisk::~WriteToDisk(void)
{
	m_bStop=true;
	int nCount=0;
	while(QThread::isRunning()){
		sleepEx(10);
		if (nCount>500&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"the thread block at:"<<m_nPosition;
		}else{
			//do nothing
		}
		nCount++;
	}
	m_pBuffer=NULL;
}

void WriteToDisk::stopWriteToDisk()
{
	m_bStop=true;
	int nCount=0;
	while(QThread::isRunning()){
		sleepEx(10);
		if (nCount>500&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"the thread block at:"<<m_nPosition;
		}else{
			//do nothing
		}
		nCount++;
	}
}

void WriteToDisk::run()
{
	m_tCheckBlockTimer.start(4000);
	bool bStop=false;
	int nStep=0;
	while(bStop==false){
		switch(nStep){
		case 0:{
			//check
			if (m_bWrite)
			{
				nStep=1;
			}else{
				if (m_bStop)
				{
					nStep=2;
				}else{
					msleep(1);
					nStep=0;
				}
			}
			   }
			   break;
		case 1:{
			//write
			bool bFlags=false;
			m_bBlock=true;
			m_nPosition=__LINE__;
			m_tBufferLock.lock();
			if (NULL!=m_pBuffer)
			{
				m_nPosition=__LINE__;
				if (ensureFileExist())
				{
					QFile tFile;
					tFile.setFileName(m_sFilePath);
					if (tFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
					{
						if (tFile.reset())
						{
							m_nPosition=__LINE__;
							quint64 uiWriteLength=tFile.write(m_pBuffer,m_uiBufferSize);
							if (uiWriteLength==m_uiBufferSize)
							{
								//do nothing
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"write buffer size unCorrect,please checkout"<<uiWriteLength<<m_uiBufferSize<<m_sFilePath;
								abort();
							}
							tFile.close();
							bFlags=true;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"over write file fail:"<<m_sFilePath;
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"can not open"<<m_sFilePath<<"please checkout,it will lose the buffer data";
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<m_sFilePath<<"can not be found and can not newly-built";
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"m_pBuffer should not been null";
				abort();
			}
			m_bBlock=false;
			m_tBufferLock.unlock();
			m_bBlock=true;
			m_nPosition=__LINE__;
			if (!bFlags)
			{
				QVariantMap tInfo;
				tInfo.insert("sFilePath",m_sFilePath);
				eventCallBack("sFilePath",tInfo);
			}else{
				//do nothing
			}
			m_bBlock=false;
			m_bWrite=false;
			   }
			   break;
		case 2:{
			//end
			bStop=true;
			   }
			   break;
		}
	}
	m_tCheckBlockTimer.stop();
}

void WriteToDisk::sleepEx( int nTime )
{
	if (m_nSleepSwitch<100)
	{
		msleep(nTime);
		m_nSleepSwitch++;
	}else{
		m_nSleepSwitch=0;
		QEventLoop tEventLoop;
		QTimer::singleShot(2,&tEventLoop,SLOT(quit()));
		tEventLoop.exec();
	}
}

void WriteToDisk::slCheckBlock()
{
	if (m_bBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"thread block at position :"<<m_nPosition<<"please checkout";
	}else{
		//do nothing
	}
}

void WriteToDisk::startWriteToDisk( char* pBuffer,QString sFilePath ,quint64 uiBufferSize)
{
	if (!QThread::isRunning())
	{
		QThread::start();
	}else{
		//do nothing
	}
	int nCount=0;
	while(!QThread::isRunning()){
		sleepEx(10);
		if (nCount>500&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"please checkout here";
		}else{
			//do nothing
		}
		nCount++;
	}
	m_tBufferLock.lock();
	m_pBuffer=pBuffer;
	m_bWrite=true;
	m_uiBufferSize=uiBufferSize;
	m_sFilePath=sFilePath;
	m_tBufferLock.unlock();
}

bool WriteToDisk::ensureFileExist()
{
	QFile tFile;
	tFile.setFileName(m_sFilePath);
	if (!tFile.exists())
	{
		QFileInfo tFileInfo(tFile);
		QString sDirPath=tFileInfo.absolutePath();
		QDir tDir;
		if (!tDir.exists(sDirPath))
		{
			if (tDir.mkpath(sDirPath))
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"create dirPath fail"<<sDirPath;
				return false;
			}
		}else{
			//keep going
		}
		if (tFile.open(QIODevice::WriteOnly))
		{
			//keep going
			tFile.close();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"create file fail:"<<m_sFilePath;
			return false;
		}
	}else{
		//do nothing
	}
	return true;
}

void WriteToDisk::registerEvent( QString sEventName,int(__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_tEventList.contains(sEventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event :"<<sEventName<<"fail";
		return;
	}else{
		tagWriteToDiskProcInfo proInfo;
		proInfo.proc=proc;
		proInfo.pUser=pUser;
		m_tEventMap.insert(sEventName,proInfo);
		return;
	}
}

void WriteToDisk::eventCallBack( QString sEventName,QVariantMap tInfo )
{
	if (m_tEventList.contains(sEventName))
	{
		tagWriteToDiskProcInfo proInfo=m_tEventMap.value(sEventName);
		if (NULL!=proInfo.proc)
		{
			proInfo.proc(sEventName,tInfo,proInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<" event is not regist";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support :"<<sEventName;
	}
}
