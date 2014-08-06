#include "SetAutoSycTime.h"

SetAutoSycTime::SetAutoSycTime(void):m_nSleepSwitch(0),
	m_bIsBlock(false),
	m_nPosition(0)
{
	m_sEventList<<"SyncTimeMsg";
	connect(&m_tCheckTimer,SIGNAL(timeout()),this,SLOT(slCheckBlock()));
	m_tCheckTimer.start(5000);
}


SetAutoSycTime::~SetAutoSycTime(void)
{
	m_bStop=true;
	int nCount=0;
	while(QThread::isRunning()){
		sleepEx(10);
		nCount++;
		if (nCount>500&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"this thread should be terminate,but it is block at position :"<<m_nPosition<<"please check";
		}else{
			//do nothing
		}
	}
	m_tCheckTimer.stop();
}

void SetAutoSycTime::setAutoSycTime(QString sAddr,quint16 uiPort,QString sUserName,QString sPassWord)
{
	if (!QThread::isRunning())
	{
		if (sAddr.isEmpty()||uiPort>65535||uiPort<0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setAutoSycTime fail as the input params is error";
		}else{
			m_tDeviceInfo.sAddr=sAddr;
			m_tDeviceInfo.uiPort=uiPort;
			m_tDeviceInfo.sUserName=sUserName;
			m_tDeviceInfo.sPassWord=sPassWord;
			m_bStop=false;
			QThread::start();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setAutoSycTime fail as the thread still running,please wait a moment";
	}
}

void SetAutoSycTime::run()
{
	int nStep=SYC_CONNECT;
	bool bRunStop=false;
	tagReceiveStepCode tReceviceStep=RECV_VERSERINFO;
	QTcpSocket *pTcpSocket=NULL;
	QByteArray tTimeZone;
	while(bRunStop==false){
		if (NULL!=pTcpSocket)
		{
			//do nothing
		}else{
			pTcpSocket=new QTcpSocket;
		}
		switch(nStep){
		case SYC_CONNECT:{
			//connect to device ,最长等待五秒的连接
			m_bIsBlock=true;
			m_nPosition=__LINE__;
			pTcpSocket->connectToHost(QHostAddress(m_tDeviceInfo.sAddr),m_tDeviceInfo.uiPort);
			if (m_bStop==false&&pTcpSocket->waitForConnected(5000))
			{
				nStep=SYC_GETVESIONINFO;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as connect to device fail";
				qDebug()<<__FUNCTION__<<__LINE__<<"error ::"<<pTcpSocket->error()<<"current state::"<<pTcpSocket->state();
				nStep=SYC_FAIL;
			}
			m_bIsBlock=false;
			}
			   break;
		case SYC_GETVESIONINFO:{
			//获取版本号,最长等待两秒，写入数据
			m_bIsBlock=true;
			m_nPosition=__LINE__;
			QByteArray block = "GET /cgi-bin/gw2.cgi?f=j&xml=%3Cjuan%20ver%3D%22%22%20seq%3D%22%22%3E%3Cconf%20type%3D%22read%22%20user%3D%22admin%22%20password%3D%22%22%3E%3Cspec%20vin%3D%22%22%20ain%3D%22%22%20io_sensor%3D%22%22%20io_alarm%3D%22%22%20hdd%3D%22%22%20sd_card%3D%22%22%20%2F%3E%3Cinfo%20device_name%3D%22%22%20device_model%3D%22%22%20device_sn%3D%22%22%20hardware_version%3D%22%22%20software_version%3D%22%22%20build_date%3D%22%22%20build_time%3D%22%22%20%2F%3E%3C%2Fconf%3E%3C%2Fjuan%3E HTTP/1.1\r\n";
			block += "Connection: Keep-Alive\r\n";
			block += "\r\n";
			pTcpSocket->write(block);
			if (m_bStop==false&&pTcpSocket->state()==QAbstractSocket::ConnectedState&&pTcpSocket->waitForBytesWritten(2000))
			{
				nStep=SYC_REVICEDATA;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as write get versionInfo data to socket fail,waiting write data more than 2 seconds";
				qDebug()<<__FUNCTION__<<__LINE__<<"error ::"<<pTcpSocket->error()<<"current state::"<<pTcpSocket->state();
				nStep=SYC_FAIL;
			}
			m_bIsBlock=false;
			   }
			   break;
		case SYC_REVICEDATA:{
			//接受数据,最长等待两秒
			m_bIsBlock=true;
			m_nPosition=__LINE__;
			if (m_bStop==false&&pTcpSocket->state()==QAbstractSocket::ConnectedState&&pTcpSocket->waitForReadyRead(2000))
			{
				if (pTcpSocket->bytesAvailable()>0)
				{
					QByteArray tBuffer=pTcpSocket->readAll();
					if (tBuffer.contains("HTTP/1.1 200 OK"))
					{
						if (tReceviceStep==RECV_VERSERINFO)
						{
							//解析版本信息
							int nPosStart=tBuffer.indexOf("software_version") + qstrlen("software_version=\\\"");
							int nPosEnd=tBuffer.indexOf("\\\"", nPosStart);
							QByteArray tMs=tBuffer.mid(nPosStart);
							QByteArray tSoftwareVersion=tBuffer.mid(nPosStart,nPosEnd-nPosStart);
							if (tSoftwareVersion.left(5)<="1.1.3")
							{
								tReceviceStep=RECV_OLDVERSER;
								nStep=SYS_SYCTIMETOOLDVERSION;
							}else{
								tReceviceStep=RECV_LOCALSYSTEMTIME;
								nStep=SYS_GETLOCALTIME;
							}

						}else if (tReceviceStep==RECV_OLDVERSER)
						{
							//接收1.1.3以前版本回复的信息
							QVariantMap tItem;
							tItem.insert("statusCode",0);
							tItem.insert("statusMsg","OK");
							eventCallBack("SyncTimeMsg",tItem);
							nStep=SYC_SUCCESS;
						}else if (tReceviceStep==RECV_LOCALSYSTEMTIME)
						{
							//接收当地时区信息
							int nPosStart=tBuffer.indexOf("\"") + 1;
							int nPosEnd=tBuffer.lastIndexOf("\"");
							QByteArray tLocalSycTime=tBuffer.mid(nPosStart,nPosEnd-nPosStart);
							tTimeZone=tLocalSycTime.right(6);
							tReceviceStep=RECV_NEWVERSER;
							nStep=SYS_SYCTIMETONEWVERSION;
						}else if (tReceviceStep==RECV_NEWVERSER)
						{
							QString sStatus;
							int nCode;
							QRegExp tRx("[a-zA-Z]+");
							if (-1!=tRx.indexIn(tBuffer,tBuffer.indexOf("statusMessage")+ qstrlen("statusMessage")))
							{
								sStatus=tRx.cap(0);
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime parse data fail";
							}
							tRx=QRegExp("\\d+");
							if (-1!=tRx.indexIn(tBuffer,tBuffer.indexOf("statusCode")+ qstrlen("statusCode")))
							{
								nCode=tRx.cap(0).toUInt();
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime parse data fail";
							}
							QVariantMap tItem;
							tItem.insert("statusCode",nCode);
							tItem.insert("statusMsg",sStatus);
							eventCallBack("SyncTimeMsg",tItem);
							nStep=SYC_SUCCESS;
							//接收1.1.3以后版本的回复信息
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as tReceviceStep code undefined"<<tReceviceStep;
							nStep=SYC_FAIL;
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as the receive data is undefined"<<tBuffer;
						nStep=SYC_FAIL;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as the receive data size is zero";
					nStep=SYC_FAIL;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as waiting receive data more than 2 seconds";
				nStep=SYC_FAIL;
			}
			m_bIsBlock=false;
							}
							break;
		case SYS_SYCTIMETOOLDVERSION:{
			m_bIsBlock=true;
			m_nPosition=__LINE__;
			QByteArray block = "GET /cgi-bin/gw2.cgi?f=j&xml=%3Cjuan%20ver%3D%22%22%20seq%3D%22%22%3E%3Cconf%20type%3D%22read%22%20user%3D%22admin%22%20password%3D%22%22%3E%3Cspec%20vin%3D%22%22%20ain%3D%22%22%20io_sensor%3D%22%22%20io_alarm%3D%22%22%20hdd%3D%22%22%20sd_card%3D%22%22%20%2F%3E%3Cinfo%20device_name%3D%22%22%20device_model%3D%22%22%20device_sn%3D%22%22%20hardware_version%3D%22%22%20software_version%3D%22%22%20build_date%3D%22%22%20build_time%3D%22%22%20%2F%3E%3C%2Fconf%3E%3C%2Fjuan%3E HTTP/1.1\r\n";
			block += "Connection: Keep-Alive\r\n";
			block += "\r\n";
			pTcpSocket->write(block);
			if (m_bStop==false&&pTcpSocket->state()==QAbstractSocket::ConnectedState&&pTcpSocket->waitForBytesWritten(2000))
			{
				nStep=SYC_REVICEDATA;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as write data to socket fail ";
				qDebug()<<__FUNCTION__<<__LINE__<<pTcpSocket->error()<<pTcpSocket->state();
				nStep=SYC_FAIL;
			}
			m_bIsBlock=false;
									 }
									 break;
		case SYS_GETLOCALTIME:{
			  m_bIsBlock=true;
			  m_nPosition=__LINE__;
	          QByteArray block;
	          block += "GET /netsdk/system/time/localtime HTTP/1.1\r\n";
	          block += "Authorization: Basic YWRtaW46\r\n";
	          block += "Accept: application/json, text/javascript, */*; q=0.01\r\n";
	          block += "X-Requested-With: XMLHttpRequest\r\n";
	          block += "Referer: http://" + m_tDeviceInfo.sAddr.toLatin1() + "/view.html\r\n";
	          block += "Host: " + m_tDeviceInfo.sAddr.toLatin1() + "\r\n";
	          block += "DNT: 1\r\n";
	          block += "Connection: Keep-Alive\r\n";
	          block += "Cookie: juanipcam_lang=zh-cn; login=admin%2C; sync_time=true; usr=" + m_tDeviceInfo.sUserName + "; pwd=" + m_tDeviceInfo.sPassWord + "\r\n";
	          block += "\r\n";
	          pTcpSocket->write(block);
	          if (m_bStop==false&&pTcpSocket->state()==QAbstractSocket::ConnectedState&&pTcpSocket->waitForBytesWritten(2000))
	          {
	          	nStep=SYC_REVICEDATA;
	          }else{
	          	qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as write data to socket fail";
	          	qDebug()<<__FUNCTION__<<__LINE__<<pTcpSocket->error()<<pTcpSocket->state();
	          	nStep=SYC_FAIL;
	          }
			  m_bIsBlock=false;
							  }
							  break;
		case SYS_SYCTIMETONEWVERSION:{
			 m_bIsBlock=true;
			 m_nPosition=__LINE__;
             QByteArray block;
             block += "PUT /netsdk/system/time/localtime HTTP/1.1\r\n";
             block += "Authorization: Basic YWRtaW46\r\n";
             block += "Accept: application/json, text/javascript, */*; q=0.01\r\n";
             block += "X-Requested-With: XMLHttpRequest\r\n";
             block += "Referer: http://" + m_tDeviceInfo.sAddr.toLatin1() + "/view.html\r\n";
             block += "Host: " + m_tDeviceInfo.sAddr.toLatin1() + "\r\n";
             block += "DNT: 1\r\n";
             block += "Content-Length: 27Connection: Keep-Alive\r\n";
             block += "Cookie: juanipcam_lang=zh-cn; login=admin%2C; sync_time=true; usr=" + m_tDeviceInfo.sUserName + "; pwd=" + m_tDeviceInfo.sPassWord + "\r\n";
             block += "\r\n";
             
             QDateTime curTime = QDateTime::currentDateTime();
             QString timeStr = curTime.toString("yyyy-MM-ddThh:mm:ss") + tTimeZone;
             block += "\"" + timeStr.toLatin1() + "\"";
             pTcpSocket->write(block);
			 if (m_bStop==false&&pTcpSocket->state()==QAbstractSocket::ConnectedState&&pTcpSocket->waitForBytesWritten(2000))
			 {
				 nStep=SYC_REVICEDATA;
			 }else{
				 qDebug()<<__FUNCTION__<<__LINE__<<"SetAutoSycTime fail as write data to socket fail";
				 qDebug()<<__FUNCTION__<<__LINE__<<pTcpSocket->error()<<pTcpSocket->state();
				 nStep=SYC_FAIL;
			 }
			 m_bIsBlock=false;
									 }
									 break;
		case SYC_FAIL:{
			//同步失败
			QVariantMap tItem;
			tItem.insert("statusCode", -1);
			tItem.insert("statusMsg", "failure");
			eventCallBack("SyncTimeMsg", tItem);
			nStep=SYC_END;
			   }
			   break;
		case SYC_SUCCESS:{
			//同步成功
			nStep=SYC_END;
						 }
						 break;
		case SYC_END:{
			bRunStop=true;
			if (NULL!=pTcpSocket)
			{
				if (pTcpSocket->state()!=QAbstractSocket::UnconnectedState)
				{
					pTcpSocket->disconnectFromHost();
					if (pTcpSocket->waitForDisconnected(2000))
					{
						//keep going
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"it cost more than 2s to disconnectFromHost,it may be cause some crash,please check";
					}
				}else{
					//do nothing
				}
				pTcpSocket->deleteLater();
			}else{
				//do nothing
			}
					 }
					 break;
		}
	}
}

void SetAutoSycTime::sleepEx(int time)
{
	if (m_nSleepSwitch<100)
	{
		msleep(time);
		m_nSleepSwitch++;
	}else{
		QEventLoop eventloop;
		QTimer::singleShot(2, &eventloop, SLOT(quit()));
		eventloop.exec();
		m_nSleepSwitch=0;
	}
	return;
}

void SetAutoSycTime::slCheckBlock()
{
	if (m_bIsBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.sAddr<<"this thread block at :"<<m_nPosition<<"pass 5s";
	}else{
		//do nothing
	}
}

void SetAutoSycTime::registerEvent( QString eventName,int (__cdecl *proc)(QString ,QVariantMap,void*),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event :"<<eventName<<"fail";
		return;
	}else{
		tagSetAutoSycTimeProInfo proInfo;
		proInfo.proc=proc;
		proInfo.pUser=pUser;
		m_tEventMap.insert(eventName,proInfo);
		return;
	}
}


void SetAutoSycTime::eventCallBack( QString sEventName,QVariantMap evMap )
{
	if (m_sEventList.contains(sEventName))
	{
		tagSetAutoSycTimeProInfo proInfo=m_tEventMap.value(sEventName);
		if (NULL!=proInfo.proc)
		{
			proInfo.proc(sEventName,evMap,proInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<"event is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support:"<<sEventName;
	}
}

