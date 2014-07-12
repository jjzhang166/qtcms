#include "remotePlayBack.h"

int cbConnectStatusRChange(QString sEventName,QVariantMap evMap,void *pUser);
int cbRecFileRFound(QString sEventName,QVariantMap evMap,void *pUser);
int cbRecSearchRFail(QString sEventName,QVariantMap evMap,void *pUser);
int cbRecSearchRFinished(QString sEventName,QVariantMap evMap,void *pUser);
int cbRecSocketRError(QString sEventName,QVariantMap evMap,void *pUser);
int cbRecRStream(QString sEventName,QVariantMap evMap,void *pUser);
remotePlayBack::remotePlayBack(void):m_bStop(true),
	m_nSleepSwitch(0),
	m_nPosition(0),
	m_uiVolumePersent(50),
	m_uiCurrentFrameTime(0),
	m_pRemotePlayback(NULL),
	m_pAudioPlayer(NULL),
	m_pCurrentWin(NULL),
	m_tPlaybackProtocol(BUBBLE),
	m_tRecConnectStatus(REC_STATUS_DISCONNECTED),
	m_bEnableAudio(false),
	m_bStreamPuase(false),
	m_bBlock(false)
{
	m_sEventNameList<<"foundFile"<<"StateChangeed"<<"recFileSearchFinished"<<"SocketError"<<"recFileSearchFail"<<"bufferStatus";
	connect(&m_tCheckBlockTimer,SIGNAL(timeout()),this,SLOT(slCheckBlock()));
	connect(this,SIGNAL(sgBackToMainThread(QString,QVariantMap)),this,SLOT(slBackToMainThread(QString,QVariantMap)));
	m_tCheckBlockTimer.start(5000);
}


remotePlayBack::~remotePlayBack(void)
{
	int nCountTime=0;
	m_bStop=true;
	while(QThread::isRunning()){
		sleepEx(10);
		nCountTime++;
		if (nCountTime>5000&&nCountTime%1000)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"terminate the work thread had caused time ::"<<nCountTime%1000<<"there may be out of control";
		}
	}
}

int remotePlayBack::setDeviceHost( const QString &sAddr )
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		return 1;
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHost fail as the thread had been running";
	}else{
		m_tRecDeviceInfo.sAddr=sAddr;
		return 0;
	}
}

int remotePlayBack::setDevicePorts( unsigned int ports )
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDevicePorts fail as the thread had been runnig";
		return 1;
	}else{
		m_tRecDeviceInfo.uiPorts=ports;
		return 0;
	}
}

int remotePlayBack::setDeviceEsee( const QString &Esee )
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceEsee fail as the thread had been running";
		return 1;
	}else{
		m_tRecDeviceInfo.sEsee=Esee;
		return 0;
	}
}

int remotePlayBack::startSearchRecFile( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime )
{
	//0:调用成功
	//1：调用失败
	//2：参数错误
	if (!QThread::isRunning())
	{
		if (nTypes<0||nTypes>15||startTime>=endTime)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as the input params error";
			return 2;
		}else{
			m_tRecDeviceInfo.nSearchChannel=nChannel;
			m_tRecDeviceInfo.nSearchTypes=nTypes;
			m_tRecDeviceInfo.tSearchStartTime=startTime;
			m_tRecDeviceInfo.tSearchEndTime=endTime;
			m_tStepCodeLock.lock();
			m_stepCode.clear();
			m_stepCode.enqueue(SEARCHRECFILE);
			m_tStepCodeLock.unlock();
			QThread::start();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as thread had been running,if you want research please wait the thread finish or call stop";
		return 1;
	}
	return 0;
}

int remotePlayBack::groupPlay( int nTypes,const QDateTime & start,const QDateTime & end )
{
	//0:成功
	//1:失败
	if (!QThread::isRunning())
	{
		if (nTypes<0||nTypes>15||start>=end)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as the input params error";
			return 2;
		}else{
			//fix me check whether the play group is isEmpty
			m_tRecDeviceInfo.nPlayTypes=nTypes;
			m_tRecDeviceInfo.tPlayStartTime=start;
			m_tRecDeviceInfo.tPlayEndTime=end;
			m_tStepCodeLock.lock();
			m_stepCode.clear();
			m_stepCode.enqueue(GROUPPLAY);
			m_tStepCodeLock.unlock();
			QThread::start();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as the thread had been running,if you want replay please call groupStop()";
	}
	return 0;
}

int remotePlayBack::groupPause()
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(GROUPPAUSE);
		m_tStepCodeLock.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread is not running ,there is no need to call this function";
		return 1;
	}
}

QDateTime remotePlayBack::getGroupPlayTime()
{
	QDateTime tTime;
	QTime tSecTime(0,0,0);
	if (QThread::isRunning()&&m_tPlayGroupMap.size()>0)
	{
		QMap<int ,tagGroupPlayInfo>::Iterator it;
		int nSeconds=m_uiCurrentFrameTime;
		it=m_tPlayGroupMap.begin();
		nSeconds-=m_tRecDeviceInfo.tPlayStartTime.toTime_t();
		tTime.setDate(QDate::currentDate());
		if (nSeconds<0)
		{
			nSeconds=0;
		}else{
			//do nothing
		}
		tTime.setTime(tSecTime.addSecs(nSeconds));
		return tTime;
	}else{
		return tTime;
	}
}

int remotePlayBack::groupContinue()
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(GROUPCONTINUE);
		m_tStepCodeLock.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread is not running ,there is no need to call this function";
		return 1;
	}
	return 0;
}

int remotePlayBack::groupStop()
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(GROUPSTOP);
		m_tStepCodeLock.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread is not running ,there is no need to call this function";
		m_tPlayGroupMap.clear();
		return 1;
	}
}

int remotePlayBack::groupSpeedFast()
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		m_tSpeedType=RECSPEEDFAST;
		m_nSpeedRate=0;
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(GROUPSPEEDFAST);
		m_tStepCodeLock.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread is not running ,there is no need to call this function";
		return 1;
	}
}

int remotePlayBack::groupSpeedSlow()
{
	//0:成功
	//1:失败

	if (QThread::isRunning())
	{
		m_tSpeedType=RECSPEEDSLOW;
		m_nSpeedRate=1;
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(GROUPSPEEDSLOW);
		m_tStepCodeLock.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread is not running ,there is no need to call this function";
		return 1;
	}
}

int remotePlayBack::groupSpeedNormal()
{
	//0:成功
	//1:失败
	if (QThread::isRunning())
	{
		m_tSpeedType=RECSPEEDNORMAL;
		m_nSpeedRate=0;
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(GROUPSPEEDNORMAL);
		m_tStepCodeLock.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread is not running ,there is no need to call this function";
		return 1;
	}
}

bool remotePlayBack::groupEnableAudio( bool bEnable )
{
	//0:成功
	//1:失败
	m_bEnableAudio=bEnable;
	if (QThread::isRunning())
	{
		m_stepCode.enqueue(GROUPENABLEAUDIO);
	}else{
		//do nothing
	}
	return true;
}

int remotePlayBack::groupSetVolume( unsigned int uiPersent, QWidget* pWnd )
{
	//0:成功
	//1:失败
	m_uiVolumePersent=uiPersent;
	m_pCurrentWin=pWnd;
	if (QThread::isRunning())
	{
		m_stepCode.enqueue(GROUPSETVOLUME);
	}else{
		//do nothing
	}
	return 0;
}

void remotePlayBack::run()
{
	int nStep=DEFAULT;
	bool bStop=false;
	m_bStop=false;
	if (NULL!=m_pRemotePlayback)
	{
		m_pRemotePlayback->Release();
		m_pRemotePlayback=NULL;
		qDebug()<<__FUNCTION__<<__LINE__<<"m_pRemotePlayback should be null,there may be some error";
	}else{
		//keep going
	}
	while(bStop==false){
		if (nStep==RECEND||nStep==GROUPSTOP)
		{
			//do nothing
		}else{
			if (!m_stepCode.isEmpty())
			{
				m_tStepCodeLock.lock();
				nStep=m_stepCode.dequeue();
				m_tStepCodeLock.unlock();
			}else{
				nStep=DEFAULT;
			}
		}
		switch(nStep){
		case SEARCHRECFILE:{
			//搜索远程录像文件
			//step 0: 初始化
			//step 1:尝试bubble协议
			//step 2:尝试穿透协议
			//step 3:尝试转发协议
			//step 4:结束
			m_nPosition=__LINE__;
			int nSearchStep=0;
			bool bSearchStop=false;
			while(bSearchStop==false){
				if (m_bStop)
				{
					nSearchStep=4;
				}else{
					//keep going
				}
				switch(nSearchStep){
				case 0:{
					//step 0: 初始化
					nSearchStep=1;
					   }
					   break;
				case 1:{
					//step 1:尝试bubble协议
					int nret=-1;
					pcomCreateInstance(CLSID_BubbleProtocol,NULL,IID_IRemotePlayback,(void**)&m_pRemotePlayback);
					if (NULL!=m_pRemotePlayback)
					{
						//注册回调函数
						//设置连接参数
						//搜索文件
						IEventRegister *pEventRegister=NULL;
						m_pRemotePlayback->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
						if (NULL!=pEventRegister)
						{
							pEventRegister->registerEvent("SocketError",cbRecSocketRError,this);
							pEventRegister->registerEvent("StateChangeed",cbConnectStatusRChange,this);
							pEventRegister->registerEvent("foundFile",cbRecFileRFound,this);
							pEventRegister->registerEvent("recFileSearchFinished",cbRecSearchRFinished,this);
							pEventRegister->registerEvent("recFileSearchFail",cbRecSearchRFail,this);
							pEventRegister->Release();
							pEventRegister=NULL;
							IDeviceConnection *pDeviceConnection=NULL;
							m_pRemotePlayback->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
							if (NULL!=pDeviceConnection)
							{
								pDeviceConnection->setDeviceAuthorityInfomation(m_tRecDeviceInfo.sUserName,m_tRecDeviceInfo.sPassword);
								pDeviceConnection->setDeviceHost(m_tRecDeviceInfo.sAddr);
								pDeviceConnection->setDeviceId(m_tRecDeviceInfo.sEsee);
								QVariantMap ports;
								ports.insert("media",m_tRecDeviceInfo.uiPorts);
								pDeviceConnection->setDevicePorts(ports);
								pDeviceConnection->Release();
								pDeviceConnection=NULL;
								m_nPosition=__LINE__;
								m_bBlock=true;
								nret=m_pRemotePlayback->startSearchRecFile(m_tRecDeviceInfo.nSearchChannel,m_tRecDeviceInfo.nSearchTypes,m_tRecDeviceInfo.tSearchStartTime,m_tRecDeviceInfo.tSearchEndTime);
								m_bBlock=false;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"bubble search file fail as it do not support IDeviceConnection interface";	
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"bubble search file fail as it do not support IEventRegister interface";
						}

					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"bubble search file fail as it do not support IRemotePlayback interface";
					}
					if (nret!=0)
					{
						nSearchStep=2;
						m_tPlaybackProtocol=BUBBLE;
					}else{
						nSearchStep=4;
					}
					if (NULL!=m_pRemotePlayback)
					{
						m_nPosition=__LINE__;
						m_bBlock=true;
						m_pRemotePlayback->Release();
						m_bBlock=false;
						m_pRemotePlayback=NULL;
					}else{
						//do nothing
					}
					   }
					   break;
				case 2:{
					//step 2:尝试穿透协议,目前不支持
					nSearchStep=3;
					   }
					   break;
				case 3:{
					//step 3:尝试转发协议，目前不支持
					nSearchStep=4;
					   }
					   break;
				case 4:{
					//step 4:结束
					bSearchStop=true;
					m_bStop=true;
					   }
					   break;
				}
			}
						   }
						   break;
		case DEALWITHSTREAM:{
			//处理码流
			m_nPosition=__LINE__;
			int nStreamStep=0;
			int nDealWithChannel=-1;
			bool bStreamStop=false;
			if (m_bPuase)
			{
				bStreamStop=true;
				sleepEx(10);
			}else{
				//do nothing
			}
			while(bStreamStop==false){
				switch(nStreamStep){
				case RECPALYINIT:{
					bool bret=true;
					QMap<int ,tagGroupPlayInfo>::Iterator it;
					for (it=m_tPlayGroupMap.begin();it!=m_tPlayGroupMap.end();it++)
					{
						if (it->tRecStreamFrame.size()<100&&it->bStartPlayStream==false)
						{
							//开始播放时，等待缓冲100帧
							sleepEx(10);
							bret=false;
							break;
						}else{
							//keep going
						}
					}
					if (bret)
					{
						nStreamStep=FINDMAXSTREAMCHANNEL;
					}else{
						nStreamStep=RECPLAYEND;
					}
								 }
								 break;
				case FINDMAXSTREAMCHANNEL:{
						QMap<int ,tagGroupPlayInfo>::Iterator it;
						int nSize=-1;
						for (it=m_tPlayGroupMap.begin();it!=m_tPlayGroupMap.end();it++)
						{
							if (it->tRecStreamFrame.size()>nSize)
							{
								nDealWithChannel=it->nChannel;
								nSize=it->tRecStreamFrame.size();
							}else{
								//do nothing
							}
						}
						QMap<int,tagGroupPlayInfo>::Iterator iter=m_tPlayGroupMap.find(nDealWithChannel);
						if (iter->tRecStreamFrame.size()<200&&m_bStreamPuase==true)
						{
							if (m_pRemotePlayback!=NULL)
							{
								m_nPosition=__LINE__;
								m_bBlock=true;
								m_pRemotePlayback->pausePlaybackStream(false);
								m_bBlock=false;
							}else{
								//do nothing
								qDebug()<<__FUNCTION__<<__LINE__<<"please check ,m_pRemotePlayback should no be null,there must be some errors";
							}
							m_bStreamPuase=false;
						}else if (iter->tRecStreamFrame.size()>1000&&m_bStreamPuase==false)
						{
							if (m_pRemotePlayback!=NULL)
							{
								m_nPosition=__LINE__;
								m_bBlock=true;
								m_pRemotePlayback->pausePlaybackStream(true);
								m_bBlock=false;
							}else{
								//do nothing
								qDebug()<<__FUNCTION__<<__LINE__<<"please check ,there must be some errors";
							}
							m_bStreamPuase=true;
						}else{
							//do nothing
						}
						if (iter->tRecStreamFrame.size()>0)
						{
							nStreamStep=RECPLAYSTREAM;
						}else{
							nStreamStep=RECPLAYEND;
						}
					   }
					   break;
				case RECPLAYSTREAM:{
						unsigned int uiLength=0;
						char *pData=NULL;
						tagRecStreamFrame tRecStreamFrame;
						memset(&tRecStreamFrame,0,sizeof(tagRecStreamFrame));
						m_nPosition=__LINE__;
						m_bBlock=true;
						m_tStreamLock.lock();
						QMap<int ,tagGroupPlayInfo>::Iterator it=m_tPlayGroupMap.find(nDealWithChannel);
						tRecStreamFrame=it->tRecStreamFrame.takeAt(0);
						m_tStreamLock.unlock();
						m_bBlock=false;
						//音频
						//视频
						int nRecPlayStreamStep=0;
						bool bRecPlayStreamStop=false;
						while(bRecPlayStreamStop==false){
							switch(nRecPlayStreamStep){
							case 0:{
								if (tRecStreamFrame.cFrameType==0)
								{
									nRecPlayStreamStep=1;
								}else if (tRecStreamFrame.cFrameType==1||2==tRecStreamFrame.cFrameType)
								{
									nRecPlayStreamStep=2;
								}else{
									nRecPlayStreamStep=3;
									qDebug()<<__FUNCTION__<<__LINE__<<"group play fail as there is a undefined stream type";
								}
								   }
								   break;
							case 1:{
								//音频
								if (NULL!=m_pAudioPlayer&&m_pCurrentWin==it->wPlayWidget)
								{
									int nSampleWidth=tRecStreamFrame.uiAudioDataWidth;
									int nSampleRate=tRecStreamFrame.uiAudioSampleRate;
									if (m_nSampleRate!=nSampleRate||m_nSampleWidth!=nSampleWidth)
									{
										m_nSampleWidth=nSampleWidth;
										m_nSampleRate=nSampleRate;
										m_pAudioPlayer->SetAudioParam(1,m_nSampleRate,m_nSampleWidth);
									}else{
										//do nothing
									}
									m_pAudioPlayer->Play(tRecStreamFrame.pData,tRecStreamFrame.uiLength);
								}else{
									//do nothing
								}
								nRecPlayStreamStep=3;
								   }
								   break;
							case 2:{
								//视频
								if (it->pVideoDecoder!=NULL&&it->pVideoRender!=NULL)
								{
									if (it->bFirstFrame)
									{
										//第一帧
										it->ui64Tsp=tRecStreamFrame.ui64TSP;
										it->pVideoDecoder->decode(tRecStreamFrame.pData,tRecStreamFrame.uiLength);
										it->bFirstFrame=false;
										it->tElapsedTimer.start();
									}else{
										//
										m_uiCurrentFrameTime=tRecStreamFrame.uiGenTime;
										int nOffSets=1000000/tRecStreamFrame.uiFrameRate;
										quint64 ui64WaitSeconds=tRecStreamFrame.ui64TSP-it->ui64Tsp-it->tElapsedTimer.nsecsElapsed()/1000+nOffSets*m_nSpeedRate;
										quint64 ui64Before=it->tElapsedTimer.nsecsElapsed()/1000;
										if (RECSPEEDFAST!=m_tSpeedType&&ui64WaitSeconds>0)
										{
											quint64 ui64Sec=ui64WaitSeconds-it->tElapsedTimer.nsecsElapsed()/1000+ui64Before-it->ui64Spend;
											usleep(ui64Sec>=0?ui64Sec:0);
										}else{
											//keep going
										}
										it->ui64Tsp=tRecStreamFrame.ui64TSP;
										it->ui64Spend=it->tElapsedTimer.nsecsElapsed()/1000-ui64Before-ui64WaitSeconds;
										it->pVideoDecoder->decode(tRecStreamFrame.pData,tRecStreamFrame.uiLength);
										it->tElapsedTimer.start();
									}
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"group play fail as it pVideoDecoder or pVideoRender is null!!! stop the thread";
									m_bStop=true;
								}
								nRecPlayStreamStep=3;
								   }
								   break;
							case 3:{
								//end
								if (NULL!=tRecStreamFrame.pData)
								{
									delete tRecStreamFrame.pData;
									tRecStreamFrame.pData=NULL;
								}else{
									//do nothing
								}
								bRecPlayStreamStop=true;
								   }
								   break;
							}
						}
						nStreamStep=RECPLAYEND;
								   }
								   break;
				case RECPLAYEND:{
					bStreamStop=true;
								}
								break;
				}
			}
							}
							break;
		case GROUPPLAY:{
			//play
			//创建协议组件
			//注册回调函数
			//连接到设备
			//申请录像码流
			//生成解码和渲染组件
			m_nPosition=__LINE__;
			bool bret=false;
			if (m_pRemotePlayback!=NULL)
			{
				m_pRemotePlayback->Release();
				m_pRemotePlayback=NULL;
				qDebug()<<__FUNCTION__<<__LINE__<<"m_pRemotePlayback should be null,there may be some errors in another position";
			}else{
				//do nothing
			}
			switch(m_tPlaybackProtocol){
			case BUBBLE:{
				pcomCreateInstance(CLSID_BubbleProtocol,NULL,IID_IRemotePlayback,(void**)&m_pRemotePlayback);
						}
						break;
			case TURN:{
				pcomCreateInstance(CLSID_Turn,NULL,IID_IRemotePlayback,(void**)&m_pRemotePlayback);
					  }
					  break;
			case HOLE:{
				pcomCreateInstance(CLSID_Hole,NULL,IID_IRemotePlayback,(void**)&m_pRemotePlayback);
					  }
					  break;
			}
			if (NULL!=m_pRemotePlayback)
			{
				IEventRegister *pEventRegister=NULL;
				m_pRemotePlayback->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
				if (NULL!=pEventRegister)
				{
					pEventRegister->registerEvent("RecordStream",cbRecRStream,this);
					pEventRegister->registerEvent("SocketError",cbRecSocketRError,this);
					pEventRegister->registerEvent("StateChangeed",cbConnectStatusRChange,this);
					pEventRegister->Release();
					pEventRegister=NULL;
					IDeviceConnection *pDeviceConnection=NULL;
					m_pRemotePlayback->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
					if (NULL!=pDeviceConnection)
					{
						pDeviceConnection->setDeviceAuthorityInfomation(m_tRecDeviceInfo.sUserName,m_tRecDeviceInfo.sPassword);
						pDeviceConnection->setDeviceHost(m_tRecDeviceInfo.sAddr);
						pDeviceConnection->setDeviceId(m_tRecDeviceInfo.sEsee);
						QVariantMap ports;
						ports.insert("media",m_tRecDeviceInfo.uiPorts);
						pDeviceConnection->setDevicePorts(ports);
						m_tRecConnectStatus=REC_STATUS_CONNECTING;
						QVariantMap evMap;
						evMap.insert("CurrentStatus",m_tRecConnectStatus);
						m_nPosition=__LINE__;
						m_bBlock=true;
						emit slBackToMainThread("CurrentStatus",evMap);
						m_bBlock=false;
						m_nPosition=__LINE__;
						m_bBlock=true;
						pDeviceConnection->connectToDevice();
						m_bBlock=false;
						pDeviceConnection->Release();
						pDeviceConnection=NULL;
						//等待是否连接上,等待5s，没有连接上 则认为连接失败
						int nCountTime=0;
						m_nPosition=__LINE__;
						m_bBlock=true;
						while(m_tRecConnectStatus!=REC_STATUS_CONNECTED&&nCountTime<500){
							sleepEx(10);
							nCountTime++;
						}
						m_bBlock=false;
						if (REC_STATUS_CONNECTED==m_tRecConnectStatus)
						{
							m_nPosition=__LINE__;
							m_bBlock=true;
							if (0==m_pRemotePlayback->getPlaybackStreamByTime(m_tRecDeviceInfo.nPlayBackChannel,m_tRecDeviceInfo.nPlayTypes,m_tRecDeviceInfo.tPlayStartTime,m_tRecDeviceInfo.tPlayEndTime))
							{
								bool ret=true;
								if (m_tPlayGroupMap.size()>0)
								{
									QMap<int ,tagGroupPlayInfo>::Iterator it;
									for (it=m_tPlayGroupMap.begin();it!=m_tPlayGroupMap.end();it++)
									{
										if (it->pVideoDecoder!=NULL)
										{
											it->pVideoDecoder->Release();
											it->pVideoDecoder=NULL;
										}else{
											//keep going
										}
										if (it->pVideoRender!=NULL)
										{
											it->pVideoRender->Release();
											it->pVideoRender=NULL;
										}else{
											//keep going
										}
										pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&it->pVideoDecoder);
										pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&it->pVideoRender);
										if (it->pVideoDecoder==NULL||it->pVideoRender==NULL)
										{
											qDebug()<<__FUNCTION__<<__LINE__<<"group fail as it can not apply for IVideoDecoder or IVideoRender interface";
											ret=false;
											break;
										}else{
											//keep going
										}
									}
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as the play group is empty,please add channel into playGroup before call groupPlay";
								}
								if (ret)
								{
									bret=true;
								}else{
									bret=false;
								}
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as getPlaybackStreamByTime fail";
							}
							m_bBlock=false;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as this protocol connect to device fail";
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as this protocol do not support IDeviceConnection interface";
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as this protocol do not support IEventRegister interface";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"groupPlay fail as there is no protocol support IRemotePlayback interface";
			}
			if (bret)
			{
				nStep=DEFAULT;
			}else{
				nStep=DEFAULT;
				m_bStop=true;
			}
					   }
					   break;
		case GROUPSTOP:{
			//停止
			m_bStop=true;
			nStep=RECEND;
			m_nPosition=__LINE__;
			if (NULL!=m_pAudioPlayer)
			{
				m_nPosition=__LINE__;
				m_bBlock=true;
				m_pAudioPlayer->Stop();
				m_bBlock=false;
				m_nPosition=__LINE__;
				m_bBlock=true;
				m_pAudioPlayer->Release();
				m_bBlock=false;
				m_pAudioPlayer=NULL;
				m_nSampleRate=0;
				m_nSampleWidth=0;
			}else{
				//do nothing
			}
			if (NULL!=m_pRemotePlayback)
			{
				if (m_tRecConnectStatus!=REC_STATUS_DISCONNECTED)
				{
					IDeviceConnection *pDeviceConnection=NULL;
					m_tRecConnectStatus=REC_STATUS_DISCONNECTING;
					QVariantMap evMap;
					evMap.insert("CurrentStatus",m_tRecConnectStatus);
					m_nPosition=__LINE__;
					m_bBlock=true;
					emit slBackToMainThread("CurrentStatus",evMap);
					m_bBlock=false;
					m_pRemotePlayback->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
					if (NULL!=pDeviceConnection)
					{
						m_nPosition=__LINE__;
						m_bBlock=true;
						pDeviceConnection->disconnect();
						m_bBlock=false;
						pDeviceConnection->Release();
						pDeviceConnection=NULL;
					}else{
						//do nothing
					}
				}else{
					//do nothing
				}
				m_nPosition=__LINE__;
				m_bBlock=true;
				m_pRemotePlayback->Release();
				m_bBlock=false;
				m_pRemotePlayback=NULL;
				if (m_tRecConnectStatus!=REC_STATUS_DISCONNECTED)
				{
					m_tRecConnectStatus=REC_STATUS_DISCONNECTED;
					QVariantMap evMap;
					evMap.insert("CurrentStatus",m_tRecConnectStatus);
					m_nPosition=__LINE__;
					m_bBlock=true;
					emit slBackToMainThread("CurrentStatus",evMap);
					m_bBlock=false;
				}else{
					//do nothing
				}
				//清除playGroup缓存
				QMap<int ,tagGroupPlayInfo>::Iterator it;
				for (it=m_tPlayGroupMap.begin();it!=m_tPlayGroupMap.end();it++)
				{
					if (it->pVideoDecoder!=NULL)
					{
						m_nPosition=__LINE__;
						m_bBlock=true;
						it->pVideoDecoder->Release();
						it->pVideoDecoder=NULL;
						m_bBlock=false;
					}else{
						//keep going
					}
					if (it->pVideoRender!=NULL)
					{
						m_nPosition=__LINE__;
						m_bBlock=true;
						it->pVideoRender->Release();
						it->pVideoRender=NULL;
						m_bBlock=false;
					}else{
						//keep going
					}
					m_nPosition=__LINE__;
					m_bBlock=true;
					tagRecStreamFrame tStreamInfo;
					while(!it->tRecStreamFrame.isEmpty()){
						tStreamInfo=it->tRecStreamFrame.takeAt(0);
						if (tStreamInfo.pData!=NULL)
						{
							delete tStreamInfo.pData;
							tStreamInfo.pData=NULL;
						}else{
							//keep going
						}
					}
					m_bBlock=false;
				}
				m_tPlayGroupMap.clear();

			}else{
				//do nothing
			}
			m_bStreamPuase=false;
			m_bPuase=false;
			m_uiCurrentFrameTime=0;
			m_nSampleRate=0;
			m_tSpeedType=RECSPEEDNORMAL;
			nStep=RECEND;
					   }
					   break;
		case GROUPENABLEAUDIO:{
			//音频开关
			m_nPosition=__LINE__;
			m_bBlock=true;
			if (m_bEnableAudio)
			{
				if (m_pAudioPlayer!=NULL)
				{
					m_pAudioPlayer->Release();
					m_pAudioPlayer=NULL;
				}else{
					//do nothing 
				}
				pcomCreateInstance(CLSID_AudioPlayer,NULL,IID_IAudioPlayer,(void**)&m_pAudioPlayer);
				if (NULL!=m_pAudioPlayer)
				{
					m_pAudioPlayer->EnablePlay(true);
					m_nSampleRate=0;
					m_nSampleWidth=0;
					
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"open audio fail as this module do not support IAudioPlayer interface";
				}
			}else{
				if (NULL!=m_pAudioPlayer)
				{
					m_pAudioPlayer->Stop();
					m_pAudioPlayer->Release();
					m_pAudioPlayer=NULL;
					m_nSampleWidth=0;
					m_nSampleRate=0;
				}
			}
			m_bBlock=false;
							  }
							  break;
		case GROUPSETVOLUME:{
			//设置音量
			m_nPosition=__LINE__;
			m_bBlock=true;
			if (NULL!=m_pAudioPlayer)
			{
				m_pAudioPlayer->SetVolume(m_uiVolumePersent);
			}else{
				//do nothing
			}
			m_bBlock=false;
							}
							break;
		case GROUPPAUSE:{
			m_bPuase=true;
						}
						break;
		case GROUPCONTINUE:{
			m_bPuase=false;
						   }
						   break;
		case GROUPSPEEDSLOW:{
			// do noting
							}
							break;
		case GROUPSPEEDNORMAL:{
			// do nothing
							  }
							  break;
		case GROUPSPEEDFAST:{
			// do noting
							}
							break;
		case DEFAULT:{
			//空操作
			m_nPosition=__LINE__;
			if (m_bStop)
			{
				// 结束
				nStep=GROUPSTOP;
			}else{
				if (m_tStreamBuffer.size()>0)
				{
					nStep=DEALWITHSTREAM;
				}else{
					nStep=DEFAULT;
					sleepEx(10);
				}
			}
					 }
					 break;
		case RECEND:{
			//结束
			m_nPosition=__LINE__;
			bStop=true;
				 }
				 break;
		}
	}
}

void remotePlayBack::registerEvent( QString eventName,int (__cdecl *proc)(QString ,QVariantMap ,void *),void *pUser )
{
	if (!m_sEventNameList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event:"<<eventName<<"fail,as the module do not support";
		return;
	}else{
		RecProcInfo tProcInfo;
		tProcInfo.proc=proc;
		tProcInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProcInfo);
		return;
	}
}

void remotePlayBack::eventCallBack( QString sEventName,QVariantMap evMap )
{
	if (m_sEventNameList.contains(sEventName))
	{
		RecProcInfo tProcInfo=m_tEventMap.value(sEventName);
		if (NULL!=tProcInfo.proc)
		{
			tProcInfo.proc(sEventName,evMap,tProcInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<" is not register,please check";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the module do not support the event of "<<sEventName;
	}
}

int remotePlayBack::addChannelIntoPlayGroup( int nChannel,QWidget * wnd )
{
	if (!QThread::isRunning())
	{
		if (m_tPlayGroupMap.size()<4&&nChannel>=0&&wnd!=NULL)
		{
			tagGroupPlayInfo tGroupPlayInfo;
			tGroupPlayInfo.pVideoDecoder=NULL;
			tGroupPlayInfo.pVideoRender=NULL;
			tGroupPlayInfo.wPlayWidget=NULL;
			tGroupPlayInfo.nChannel=-1;
			tGroupPlayInfo.bStartPlayStream=false;
			tGroupPlayInfo.bFirstFrame=true;
			tGroupPlayInfo.ui64Tsp=0;
			tGroupPlayInfo.ui64Spend=0;
			tGroupPlayInfo.tRecStreamFrame.clear();
			if (1!=((m_tRecDeviceInfo.nPlayBackChannel>>nChannel)&1))
			{
				m_tRecDeviceInfo.nPlayBackChannel|=1<<nChannel;
				tGroupPlayInfo.nChannel=nChannel;
				remoteRePeatWnd(wnd);
				tGroupPlayInfo.wPlayWidget=wnd;
				m_tPlayGroupMap.insert(nChannel,tGroupPlayInfo);
			}else{
				m_tPlayGroupMap[nChannel].wPlayWidget=wnd;
			}
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"addChannelIntoPlayGroup fail as input params error or playGroup had been more than four";
		}
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"addChannelIntoPlayGroup fail as the work thread han been running";
	}
	return 1;
}

int remotePlayBack::cbConnectStatusChange( QString sEventName,QVariantMap evMap,void *pUser )
{
	//eventCallBack(sEventName,evMap);
	emit sgBackToMainThread("CurrentStatus",evMap);
	return 0;
}

int remotePlayBack::cbRecFileFound( QString sEventName,QVariantMap evMap,void *pUser )
{
	eventCallBack(sEventName,evMap);
	return 0;
}

int remotePlayBack::cbRecSearchFail( QString sEventName,QVariantMap evMap,void *pUser )
{
	eventCallBack(sEventName,evMap);
	return 0;
}

int remotePlayBack::cbRecSearchFinished( QString sEventName,QVariantMap evMap,void *pUser )
{
	eventCallBack(sEventName,evMap);
	return 0;
}

int remotePlayBack::cbRecSocketError( QString sEventName,QVariantMap evMap,void *pUser )
{
	eventCallBack(sEventName,evMap);
	return 0;
}

int remotePlayBack::cbRecStream( QString sEventName,QVariantMap evMap,void *pUser )
{
	if (!m_bStop&&!evMap.isEmpty())
	{
		//保存码流
		int nChannel=evMap.value("channel").toInt();	
		tagRecStreamFrame tRecStreamInfo;
		int nFrameType=evMap.value("frametype").toInt();
		if (0==nFrameType)
		{
			tRecStreamInfo.uiLength=evMap.value("length").toUInt();
			tRecStreamInfo.cFrameType=nFrameType;
			tRecStreamInfo.cChannel=evMap.value("channel").toUInt();
			tRecStreamInfo.uiAudioSampleRate=evMap.value("audioSampleRate").toUInt();
			tRecStreamInfo.uiAudioDataWidth=evMap.value("audioDataWidth").toUInt();
			tRecStreamInfo.ui64TSP=evMap.value("pts").toULongLong();
			tRecStreamInfo.uiGenTime=evMap.value("gentime").toUInt();
			tRecStreamInfo.pData=new char[tRecStreamInfo.uiLength];
			memcpy(tRecStreamInfo.pData,(char*)evMap.value("data").toUInt(),tRecStreamInfo.uiLength);
		}else if (1==nFrameType||2==nFrameType)
		{
			tRecStreamInfo.uiLength = evMap.value("length").toUInt();
			tRecStreamInfo.cFrameType = evMap.value("frametype").toUInt();
			tRecStreamInfo.cChannel = evMap.value("channel").toUInt();
			tRecStreamInfo.uiWidth = evMap.value("width").toInt();
			tRecStreamInfo.uiHeight = evMap.value("height").toUInt();
			tRecStreamInfo.uiFrameRate = evMap.value("framerate").toUInt();
			tRecStreamInfo.ui64TSP = evMap.value("pts").toULongLong();
			tRecStreamInfo.uiGenTime = evMap.value("gentime").toUInt();
			tRecStreamInfo.pData = new char[tRecStreamInfo.uiLength];
			memcpy(tRecStreamInfo.pData, (char*)evMap["data"].toUInt(), tRecStreamInfo.uiLength);
		}else{
			//do nothing
			qDebug()<<__FUNCTION__<<__LINE__<<"there is a undefined stream type,please check it ";
			return 0;
		}
		m_tStreamLock.lock();
		QMap<int ,tagGroupPlayInfo>::Iterator iter=m_tPlayGroupMap.find(nChannel);
		iter->tRecStreamFrame.enqueue(tRecStreamInfo);
		//帧数小于100，抛出现存 帧数
		//帧数大于100，即开始播放
		//帧数小于200，申请码流Continue
		//帧数大于1000，即申请暂停码流Pause
		if (iter->bStartPlayStream==false)
		{
			if (iter->tRecStreamFrame.size()<=100)
			{
				QVariantMap item;
				int nWind=(int)iter->wPlayWidget;
				item.insert("Persent", iter->tRecStreamFrame.size());
				item.insert("wind", nWind);
				eventCallBack("bufferStatus",item);
			}else{
				iter->bStartPlayStream=true;
			}
		}else{
			//do nothing
		}
		m_tStreamLock.unlock();
		return 0;
	}else{
		return 0;
	}
}

void remotePlayBack::sleepEx( int time )
{
	if (m_nSleepSwitch<100)
	{
		msleep(time);
		m_nSleepSwitch++;
	}else{
		QEventLoop eventloop;
		QTimer::singleShot(2,&eventloop,SLOT(quit()));
		eventloop.exec();
		m_nSleepSwitch=0;
	}
}

int remotePlayBack::checkUser( const QString &sUserName,const QString& sPassword )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"checkUser fail as the thread had been running";
		return 1;
	}else{
		m_tRecDeviceInfo.sUserName=sUserName;
		m_tRecDeviceInfo.sPassword=sPassword;
		return 0;
	}
}

void remotePlayBack::slCheckBlock()
{
	if (m_bBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"work thread block at ::"<<m_nPosition;
	}else{
		//do nothing
	}
}

void remotePlayBack::slBackToMainThread( QString sEventName,QVariantMap evMap )
{
	if (sEventName=="CurrentStatus")
	{
		if (evMap.contains("CurrentStatus"))
		{
			m_tRecConnectStatus=(tagRemovePlayBackConnectStatus)evMap.value("CurrentStatus").toInt();
			switch(m_tRecConnectStatus){
			case REC_STATUS_DISCONNECTED:{
				if (m_tRecConnectStatus!=m_tRecHisConnectStatus)
				{
					m_bStop=true;
				}else{
					//do noting
				}
										 }
										 break;
			case REC_STATUS_DISCONNECTING:{

										  }
										  break;
			case REC_STATUS_CONNECTED:{
				if (m_tRecHisConnectStatus!=m_tRecConnectStatus)
				{
					//音频
					m_stepCode.enqueue(GROUPENABLEAUDIO);
					m_stepCode.enqueue(GROUPSETVOLUME);
				}else{
					// do nothing
				}
									  }
									  break;
			case REC_STATUS_CONNECTING:{

									   }
									   break;
			}
			if (m_tRecHisConnectStatus!=m_tRecConnectStatus)
			{
				//状态转变，回调
				eventCallBack("StateChangeed",evMap);
			}else{
				//do nothing
			}
			m_tRecHisConnectStatus=m_tRecConnectStatus;
		}else{
			//do nothing
		}
	}
}

void remotePlayBack::remoteRePeatWnd( QWidget *wWin )
{
	QMap<int ,tagGroupPlayInfo>::Iterator it;
	for (it=m_tPlayGroupMap.begin();it!=m_tPlayGroupMap.end();it++)
	{
		if (it->wPlayWidget==wWin)
		{
			int temp=~m_tRecDeviceInfo.nPlayBackChannel;
			temp|=1<<(it.key()-1);
			m_tRecDeviceInfo.nPlayBackChannel=~temp;
			m_tPlayGroupMap.remove(it.key());
		}else{
			//do noting
		}
	}
}

int cbConnectStatusRChange( QString sEventName,QVariantMap evMap,void *pUser )
{
	return ((remotePlayBack*)pUser)->cbConnectStatusChange(sEventName,evMap,pUser);
}

int cbRecFileRFound( QString sEventName,QVariantMap evMap,void *pUser )
{
	return ((remotePlayBack*)pUser)->cbRecFileFound(sEventName,evMap,pUser);
}

int cbRecRStream( QString sEventName,QVariantMap evMap,void *pUser )
{
	return ((remotePlayBack*)pUser)->cbRecStream(sEventName,evMap,pUser);
}

int cbRecSocketRError( QString sEventName,QVariantMap evMap,void *pUser )
{
	return ((remotePlayBack*)pUser)->cbRecSocketError(sEventName,evMap,pUser);
}

int cbRecSearchRFail( QString sEventName,QVariantMap evMap,void *pUser )
{
	return ((remotePlayBack*)pUser)->cbRecSearchFail(sEventName,evMap,pUser);
}

int cbRecSearchRFinished( QString sEventName,QVariantMap evMap,void *pUser )
{
	return ((remotePlayBack*)pUser)->cbRecSearchFinished(sEventName,evMap,pUser);
}
