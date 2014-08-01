#include "bubbleprotocolex.h"
#include <guid.h>
#include <QtEndian>
BubbleProtocolEx::BubbleProtocolEx():m_nRef(0),
	m_nSleepSwitch(0),
	m_bStop(true),
	m_bBlock(false),
	m_tCurrentConnectStatus(BUBBLE_DISCONNECTED),
	m_tHistoryConnectStatus(BUBBLE_DISCONNECTED),
	m_pTcpSocket(NULL)
{
	m_sEventList<<"LiveStream"           <<"SocketError"<<"StateChangeed"<<"foundFile"
		<<"recFileSearchFinished"<<"RecordStream"  <<"SocketError"  <<"StateChanged"<<"recFileSearchFail"<<"ConnectRefuse";
	connect(this,SIGNAL(sgBackToMainThread(QVariantMap)),this,SLOT(slBackToMainThread(QVariantMap)),Qt::BlockingQueuedConnection);
	connect(&m_tCheckoutBlockTimer,SIGNAL(timeout()),this,SLOT(slCheckoutBlock()));
	m_tCheckoutBlockTimer.start(5000);
}

BubbleProtocolEx::~BubbleProtocolEx()
{
	m_bStop=true;
	int nCount=0;
	while(QThread::isRunning()){
		sleepEx(10);
		nCount++;
		if (nCount>500&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_tDeviceInfo.sEseeId<<"terminate this thread had caused more time than 5s,there may be out of control,"<<"please check position at:"<<m_nPosition;
		}
	}
	m_tCheckoutBlockTimer.stop();
}

long __stdcall BubbleProtocolEx::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase==iid)
	{
		*ppv=static_cast<IPcomBase *>(this);
	}else if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister *>(this);
	}else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall BubbleProtocolEx::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall BubbleProtocolEx::Release()
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

QStringList BubbleProtocolEx::eventList()
{
	return m_sEventList;
}

int BubbleProtocolEx::queryEvent( QString eventName,QStringList& eventParams )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		//fix eventParams
		return IEventRegister::OK;
	}
}

int BubbleProtocolEx::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagBubbleProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return IEventRegister::OK;
	}
}

void BubbleProtocolEx::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_sEventList.contains(sEvent))
	{
		tagBubbleProInfo tProInfo=m_tEventMap.value(sEvent);
		if (NULL!=tProInfo.proc)
		{
			tProInfo.proc(sEvent,tInfo,tProInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is  undefined";
	}
}

void BubbleProtocolEx::run()
{
	int nRunStep=BUBBLE_RUN_CONNECT;
	bool bRunStop=false;
	int nHeartBeat=0;
	QVariantMap evMap;
	m_tCurrentConnectStatus=BUBBLE_CONNECTTING;
	evMap.insert("status",m_tCurrentConnectStatus);
	sgBackToMainThread(evMap);
	while(bRunStop==false){
		switch(nRunStep){
		case BUBBLE_RUN_CONNECT:{
			//连接到设备
			if (NULL!=m_pTcpSocket)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"m_pTcpSocket should be null on here,please check";
				delete m_pTcpSocket;
				m_pTcpSocket=NULL;
			}else{
				//keep going  
			}
			m_pTcpSocket=new QTcpSocket;
			m_pTcpSocket->connectToHost(m_tDeviceInfo.tIpAddr.toString(),m_tDeviceInfo.tPorts["media"].toInt());
			//等待5s，如果超过5s没有返回，则认为连接失败
			if (m_pTcpSocket->waitForConnected(5000))
			{
				//套接字连接成功
				//下一步验证bubble是否支持使用http协议层来交互
				QString sBlock="GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
				m_pTcpSocket->write(sBlock.toAscii());
				//等待2s，如果超过2s写入套接字失败，则断开连接
				if (m_pTcpSocket->waitForBytesWritten(2000))
				{
					//等待验证返回,如果一直处于连接状态，但没有任何信息返回，如何处理
					nRunStep=BUBBLE_RUN_DEFAULT;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_pTcpSocket->state()<<m_pTcpSocket->error()<<"write block to m_pTcpSocket fail";
					nRunStep=BUBBLE_RUN_DISCONNECT;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_pTcpSocket->state()<<m_pTcpSocket->error()<<"connect to device fail";
				nRunStep=BUBBLE_RUN_DISCONNECT;
			}
								}
								break;
		case BUBBLE_RUN_RECEIVE:{
			//接受解析码流
			int nReceiveStep=BUBBLE_RECEIVE_HTTP;
			bool bReceiveStop=false;
			m_tBuffer+=m_pTcpSocket->readAll();
			while(bReceiveStop==false){
				switch(nReceiveStep){
				case BUBBLE_RECEIVE_HTTP:{
					//检测bubble是否能通过http传输
					//检测m_tBuffer 是否包含HTTP/1.1 200 或者 HTTP/1.1 404
					//忽略 m_tBuffer 大小小于12的情况
					if (m_tBuffer.contains("HTTP/1.1 200")||m_tBuffer.contains("HTTP/1.1 404"))
					{
						//检测是否够1024个字节
						if (m_tBuffer.size()<1024)
						{
							//接着接收
							nReceiveStep=BUBBLE_RECEIVE_WAITMOREBUFFER;
						}else{
							//解析
							if (analyzeBubbleInfo())
							{
								//keep going
								nReceiveStep=BUBBLE_RECEIVE_FRAME;
							}else{
								//解析失败，断开连接
								nReceiveStep=BUBBLE_RECEIVE_END;
							}
						}
					}else{
						nReceiveStep=BUBBLE_RECEIVE_FRAME;
					}
					   }
					   break;
				case BUBBLE_RECEIVE_FRAME:{
					//解析数据帧,第一次解析，buffer 大小不小于11
					//step1：查找帧头
					//step2:判断数据帧大小，超出500k，认为数据不合法，断开连接；等待buffer长度满足数据帧的大小
					//step3:查找指令码，如果没有相应的指令码，认为数据不合法，断开连接
					int nFrameStep=0;
					bool bFrameStop=false;
					while(bFrameStop==false){
						switch(nFrameStep){
						case 0:{
							//判断
							if (m_tBuffer.size()>11)
							{
								if (m_tBuffer.startsWith("\xab")||m_tBuffer.startsWith("\xaa"))
								{
									//step1:find帧头
									tagBubbleInfo *pBubbleInfo=(tagBubbleInfo *)m_tBuffer.data();
									int nBubbleSize=qToBigEndian(pBubbleInfo->uiLength)+sizeof(pBubbleInfo->cHead)+sizeof(pBubbleInfo->uiLength);
									if (nBubbleSize<1024*500)
									{
										//判断指令码
										if (m_tBuffer.startsWith("\xab"))
										{
											//回放
											if (m_tRemoteCode.contains(pBubbleInfo->cCmd))
											{
												if (m_tBuffer.size()>=nBubbleSize)
												{
													nFrameStep=1;
												}else{
													nFrameStep=2;
												}
											}else{
												//没有查询到相关的指令码，判定为解析失败
												qDebug()<<__FUNCTION__<<__LINE__<<"analysis data fail as it(pBubbleInfo->cCmd) do not match any cmd code";
												nFrameStep=3;
											}	
										}else{
											//预览
											if (m_tPreviewCode.contains(pBubbleInfo->cCmd))
											{
												if (m_tBuffer.size()>=nBubbleSize)
												{
													nFrameStep=1;
												}else{
													nFrameStep=2;
												}
											}else{
												//没有查询到相关的指令码，判定为解析失败
												qDebug()<<__FUNCTION__<<__LINE__<<"analysis data fail as it(pBubbleInfo->cCmd) do not match any cmd code";
												nFrameStep=3;
											}
										}
										pBubbleInfo->cCmd;
									}else{
										//数据帧大于500k，解析为错误数据，断开连接
										qDebug()<<__FUNCTION__<<__LINE__<<"analysis data fail as the buffer size is over 500k,we will reConnect the device";
										nFrameStep=3;
									}
								}else{
									m_tBuffer.remove(0,1);
									nFrameStep=0;
								}
							}else{
								nFrameStep=2;
							}
							   }
							   break;
						case 1:{
							//find
							if (m_tBuffer.startsWith("\xab"))
							{
								//解析回放数据
								if (analyzeRemoteInfo())
								{
									nFrameStep=2;
								}else{
									nFrameStep=3;
								}
							}else{
								//解析预览数据
								if (analyzePreviewInfo())
								{
									nFrameStep=2;
								}else{
									nFrameStep=3;
								}
							}
							   }
							   break;
						case 2:{
							//需要接着接收数据
							nFrameStep=4;
							nReceiveStep=BUBBLE_RECEIVE_WAITMOREBUFFER;
							   }
							   break;
						case 3:{
							//解析失败
							nFrameStep=4;
							nReceiveStep=BUBBLE_RECEIVE_END;
							   }
							   break;
						case 4:{
							//end
							bFrameStop=true;
							   }
							   break;
						}
					}
					   }
					   break;
				case BUBBLE_RECEIVE_WAITMOREBUFFER:{
					//等待更多的数据
					nRunStep=BUBBLE_RUN_DEFAULT;
					bReceiveStop=true;
					   }
					   break;
				case BUBBLE_RECEIVE_END:{
					//停止接收数据
					nRunStep=BUBBLE_RUN_DISCONNECT;
					bReceiveStop=true;
										}
										break;
				}
			}
								}
								break;
		case BUBBLE_RUN_CONTROL:{
			//控制指令
			m_csStepCode.lock();
			int nRunControlStep=m_tStepCode.dequeue();
			m_csStepCode.unlock();
			bool bFlag=false;
			switch(nRunControlStep){
			case BUBBLE_AUTHORITY:{
				//用户验证
				bFlag=cmdAuthority();
								  }
								  break;
			case BUBBLE_DISCONNECT:{
				//断开连接
				bFlag=cmdDisConnect();
								   }
								   break;
			case BUBBLE_GETLIVESTREAM:{
				//获取预览码流
				if (m_bIsSupportHttp)
				{
					bFlag=cmdGetLiveStreamEx();
				}else{
					bFlag=cmdGetLiveStream();
				}
				
									  }
									  break;
			case BUBBLE_STOPSTREAM:{
				//停止预览
				bFlag=cmdStopStream();
								   }
								   break;
			case BUBBLE_PAUSESTREAM:{
				//暂停预览
				bFlag=cmdPauseStream();
									}
									break;
			case BUBBLE_HEARTBEAT:{
				//心跳指令
				bFlag=cmdHeartBeat();
								  }
								  break;
			case BUBBLE_GETPLAYBACKSTREAMBYTIME:{
				//获取回放码流
				bFlag=cmdGetPlayBackStreamByTime();
												}
												break;
			case BUBBLE_GETPLAYBACKSTREAMBYFILENAME:{
				//获取回放码流
				bFlag=cmdGetPlayBackStreamByFileName();
													}
													break;
			case BUBBLE_PAUSEPLAYBACKSTREAM:{
				//暂停回放码流
				bFlag=cmdPausePlayBackStream();
											}
											break;
			case BUBBLE_STOPPLAYBACKSTREAM:{
				//停止回放码流
				bFlag=cmdStopPlayBackStream();
										   }
										   break;
			case BUBBLE_PTZ:{
				//云台控制
				bFlag=cmdPtz();
							}
							break;
			default:{
				qDebug()<<__FUNCTION__<<__LINE__<<"there is an undefined cmd,please check! it will cause device reConnect";
					}
			}
			if (bFlag)
			{
				nRunStep=BUBBLE_RUN_DISCONNECT;
			}else{
				nRunStep=BUBBLE_RUN_DEFAULT;
			}
			
								}
								break;
		case BUBBLE_RUN_DISCONNECT:{
			//断开连接
			if (m_tCurrentConnectStatus==BUBBLE_CONNECTTING||m_tCurrentConnectStatus==BUBBLE_CONNECTED)
			{
				m_tCurrentConnectStatus=BUBBLE_DISCONNECTING;
				QVariantMap evMap;
				evMap.insert("status",evMap);
				sgBackToMainThread(evMap);
			}else{
				//do nothing
			}
			//断开套接字连接的信号

			//断开套接字的网络连接
			if (NULL!=m_pTcpSocket)
			{
				if (QAbstractSocket::UnconnectedState!=m_pTcpSocket->state())
				{
					m_pTcpSocket->disconnectFromHost();
					//等待2s。超过2s没有断开返回，给出错误的提示信息
					if (m_pTcpSocket->waitForDisconnected(2000))
					{
						//成功断开
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<"try to disconnectFromHost,but it had cost 2s,please check";
					}
				}else{
					//do nothing
				}
				
			}else{
				//do nothing
			}
			nRunStep=BUBBLE_RUN_END;
								   }
								   break;
		case BUBBLE_RUN_DEFAULT:{
			//缺省动作
			//检测是否需要停止线程
			if (!m_bStop&&NULL!=m_pTcpSocket)
			{
				//检测连接状态
				if (QAbstractSocket::ConnectedState==m_pTcpSocket->state())
				{
					//指令优先于接受码流
					if (m_tStepCode.size()>0)
					{
						nRunStep=BUBBLE_RUN_CONTROL;
						sleepEx(10);
					}else{
						if (m_pTcpSocket->bytesAvailable()>0)
						{
							nRunStep=BUBBLE_RUN_RECEIVE;
						}else{
							sleepEx(10);
							nRunStep=BUBBLE_RUN_DEFAULT;
						}
					}
					//三分钟发射一次心脏指令
					nHeartBeat++;
					if (nHeartBeat>3000)
					{
						nHeartBeat=0;
						m_csStepCode.lock();
						m_tStepCode.enqueue(BUBBLE_HEARTBEAT);
						m_csStepCode.unlock();
					}else{
						//do nothing
					}
				}else{
					nRunStep=BUBBLE_RUN_DISCONNECT;
					qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<"turn to disconnect as current state::"<<m_pTcpSocket->state()<<m_pTcpSocket->error();
				}
			}else{
				nRunStep=BUBBLE_RUN_DISCONNECT;
				qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<"turn to disconnect as m_bStop::"<<m_bStop;
			}
								}
								break;
		case BUBBLE_RUN_END:{
			//结束
			bRunStop=true;
			if (NULL!=m_pTcpSocket)
			{
				m_pTcpSocket->deleteLater();
			}else{
				//do nothing
			}
							}
							break;
		}
	}
	evMap.clear();
	m_tCurrentConnectStatus=BUBBLE_DISCONNECTED;
	evMap.insert("status",m_tCurrentConnectStatus);
	sgBackToMainThread(evMap);
}

int BubbleProtocolEx::setDeviceHost( const QString &sIpAddr )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		if (m_tDeviceInfo.tIpAddr.setAddress(sIpAddr))
		{
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHost fail as the input ip addr is illegal:"<<sIpAddr;
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHost fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::setDevicePorts( const QVariantMap &tPorts )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		if (tPorts["media"].toInt()<=0||tPorts["media"].toInt()>=65535)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDevicePorts fail as the input ports is illegal :"<<tPorts["media"].toInt();
			return 1;
		}else{
			m_tDeviceInfo.tPorts=tPorts;
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDevicePorts fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::setDeviceId( const QString &sEseeId )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		m_tDeviceInfo.sEseeId=sEseeId;
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceId fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::setDeviceAuthorityInfomation( QString sUserName,QString sPassword )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		m_tDeviceInfo.sPassword=sPassword;
		m_tDeviceInfo.sUserName=sUserName;
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceAuthorityInfomation fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::connectToDevice()
{
	//0：连接成功
	//1：连接失败
	if (!QThread::isRunning())
	{
		m_csStepCode.lock();
		m_tStepCode.clear();
		m_csStepCode.unlock();
		QThread::start();

	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"connectToDevice fail ,if you want to connect to device ,please wait last connect thread terminate";
		return 1;
	}
	return 0;
}

int BubbleProtocolEx::authority()
{
	//	0:校验成功
	//	1:校验失败
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_AUTHORITY);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"authority fail as the current connect status is not in connected";
		return 1;
	}
	return 0;
}

int BubbleProtocolEx::disconnect()
{
	//0：断开成功
	//1：断开失败
	if (QThread::isRunning())
	{
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_DISCONNECT);
		m_csStepCode.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"it had been in disconnect status,there is no need to call this function again";
	}
	return 0;
}

int BubbleProtocolEx::getCurrentStatus()
{
	return m_tCurrentConnectStatus;
}

QString BubbleProtocolEx::getDeviceHost()
{
	return m_tDeviceInfo.tIpAddr.toString();
}

QString BubbleProtocolEx::getDeviceid()
{
	return m_tDeviceInfo.sEseeId;
}

QVariantMap BubbleProtocolEx::getDevicePorts()
{
	return m_tDeviceInfo.tPorts;
}

int BubbleProtocolEx::getLiveStream( int nChannel,int nStream )
{
	//0：获取成功
	//1：获取失败
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_tDeviceInfo.nPreChannel=nChannel;
		m_tDeviceInfo.nPreStream=nStream;
		//fix me
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getLiveStream fail as the current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::stopStream()
{
	//0：断开成功
	//1：断开失败
	if (QThread::isRunning())
	{
		//fix me
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread had beed terminte,there is no need to call this function";
	}
	return 0;
}

int BubbleProtocolEx::pauseStream( bool bPause )
{
	//0：暂停成功
	//1：暂停失败
	//fix me
	return 0;
}

int BubbleProtocolEx::getStreamCount()
{
	//fix me
	return 0;
}

int BubbleProtocolEx::getStreamInfo( int nStreamId,QVariantMap &tStreamInfo )
{
	//fix me
	return 0;
}

int BubbleProtocolEx::startSearchRecFile( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime )
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	//fix me
	return 0;
}

int BubbleProtocolEx::getPlaybackStreamByTime( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime )
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	//fix me
	return 0;
}

int BubbleProtocolEx::getPlaybackStreamByFileName( int nChannel,const QString &sFileName )
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	//fix me
	return 0;
}

int BubbleProtocolEx::pausePlaybackStream( bool bPause )
{
	//	0:调用成功
	//	1:调用失败
	//fix me
	return 0;
}

int BubbleProtocolEx::stopPlaybackStream()
{
	//	0:调用成功
	//	1:调用失败
	//fix me
	return 0;
}

void BubbleProtocolEx::sleepEx( int nTime )
{
	if (m_nSleepSwitch<100)
	{
		msleep(nTime);
		m_nSleepSwitch++;
	}else{
		QEventLoop tEventLoop;
		QTimer::singleShot(2,&tEventLoop,SLOT(quit()));
		tEventLoop.exec();
		m_nSleepSwitch=0;
	}
	return;
}

void BubbleProtocolEx::slCheckoutBlock()
{
	if (m_bBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_tDeviceInfo.sEseeId<<"block at ::"<<m_nPosition<<"please check!";
	}else{
		//keep going
	}
}

void BubbleProtocolEx::slBackToMainThread( QVariantMap evMap )
{
	if (evMap.contains("status"))
	{
		//状态改变，抛出状态
		if (evMap.value("status").toInt()!=m_tHistoryConnectStatus)
		{
			eventProcCall("StateChangeed",evMap);
			m_tHistoryConnectStatus=(tagBubbleConnectStatusInfo)evMap.value("status").toInt();
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
}

bool BubbleProtocolEx::analyzeBubbleInfo()
{
	return true;
}

bool BubbleProtocolEx::analyzePreviewInfo()
{
	return true;
}

bool BubbleProtocolEx::analyzeRemoteInfo()
{
	return true;
}

bool BubbleProtocolEx::cmdAuthority()
{
	char cBuffer[100];
	QByteArray tBlock;
	qint64 nLen=0;
	tagBubbleMessageInfo *pMessageInfo=NULL;
	tagBubbleAuthoritySend *pAuthoritySend=NULL;
	memset(cBuffer,0,100);
	tagBubbleInfo *pBubbleInfo=(tagBubbleInfo*)cBuffer;
	pBubbleInfo->cHead=(char)0xaa;

	QDateTime tTime=QDateTime::currentDateTime();
	pBubbleInfo->uiTicket=qToBigEndian((unsigned int)tTime.toMSecsSinceEpoch()*1000);
	pBubbleInfo->cCmd=(char)0x00;

	pMessageInfo=(tagBubbleMessageInfo*)pBubbleInfo->pLoad;
	pMessageInfo->cMessage=(char)0x00;
	memset(pMessageInfo->cReverse,(char)0x00,3);

	pAuthoritySend=(tagBubbleAuthoritySend*)pMessageInfo->cParameters;
	qstrcpy(pAuthoritySend->cUserName,m_tDeviceInfo.sUserName.toUtf8().data());
	qstrcpy(pAuthoritySend->cPassWord,m_tDeviceInfo.sPassword.toUtf8().data());

	return true;
}

bool BubbleProtocolEx::cmdDisConnect()
{
	return true;
}

bool BubbleProtocolEx::cmdGetLiveStream()
{
	return true;
}

bool BubbleProtocolEx::cmdGetLiveStreamEx()
{
	return true;
}

bool BubbleProtocolEx::cmdStopStream()
{
	return true;
}

bool BubbleProtocolEx::cmdPauseStream()
{
	return true;
}

bool BubbleProtocolEx::cmdHeartBeat()
{
	return true;
}

bool BubbleProtocolEx::cmdGetPlayBackStreamByTime()
{
	return true;
}

bool BubbleProtocolEx::cmdGetPlayBackStreamByFileName()
{
	return true;
}

bool BubbleProtocolEx::cmdPausePlayBackStream()
{
	return true;
}

bool BubbleProtocolEx::cmdPtz()
{
	return true;
}

bool BubbleProtocolEx::cmdStopPlayBackStream()
{
	return true;
}
