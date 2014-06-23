#include "QSubviewRun.h"

int cbConnectRState(QString evName,QVariantMap evMap,void *pUser);
int cbPreviewRData(QString evName,QVariantMap evMap,void *pUser);
int cbRecorderRData(QString evName,QVariantMap evMap,void*pUser);
int cbConnectRError(QString evName,QVariantMap evMap,void*pUser);
int cbDecodeRFrame(QString evName,QVariantMap evMap,void*pUser);
int cbRecordRState(QString evName,QVariantMap evMap,void*pUser);
bool QSubviewRun::m_bIsAudioOpen=false;
unsigned int QSubviewRun::m_volumePersent=50;
QSubviewRun::QSubviewRun(void):m_pdeviceClient(NULL),
	m_currentStatus(STATUS_DISCONNECTED),
	m_historyStatus(STATUS_DISCONNECTED),
	m_pIVideoDecoder(NULL),
	m_pIVideoRender(NULL),
	m_pRecorder(NULL),
	m_stop(false),
	m_bIsPtzAutoOpen(false),
	m_bIsAutoRecording(false),
	m_bIsRecord(false),
	m_bIsdataBaseFlush(true),
	m_bIsSysTime(false),
	m_bIsFocus(false),
	m_bScreenShot(false),
	m_pAudioPlay(NULL),
	m_sampleWidth(0),
	m_sampleRate(0),
	m_nInitHeight(0),
	m_nInitWidth(0)
{
	connect(this,SIGNAL(sgstopPreview()),this,SLOT(slstopPreview()),Qt::BlockingQueuedConnection);
	connect(this,SIGNAL(sgbackToMainThread()),this,SLOT(slbackToMainThread()),Qt::BlockingQueuedConnection);
	connect(&m_planRecordTimer,SIGNAL(timeout()),this,SLOT(slplanRecord()),Qt::BlockingQueuedConnection);
	m_eventNameList<<"LiveStream"<<"SocketError"<<"CurrentStatus"<<"ForRecord"<<"RecordState"<<"DecodedFrame";
}


QSubviewRun::~QSubviewRun(void)
{
	m_stop=true;
	int n=0;
	while(QThread::isRunning()){
		msleep(10);
		n++;
		if (n>500&&n%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"terminate this thread had caused more time than 5s,there may be out of control";
		}
	}
}

void QSubviewRun::run()
{
	//此函数内生成的资源，必须仅在此函数内销毁
	
	m_stop=false;
	m_bIsRecord=false;
	m_bIsAutoRecording=false;
	m_nInitHeight=0;
	m_nInitWidth=0;

	m_currentStatus=STATUS_CONNECTING;
	QVariantMap curStatusInfo;
	curStatusInfo.insert("CurrentStatus",m_currentStatus);
	emit sgbackToMainThread(curStatusInfo);
	int nstep=DEFAULT;
	bool nstop=false;
	while(!nstop){
		if (!m_stepCode.isEmpty())
		{
			nstep=m_stepCode.dequeue();
		}else{
			msleep(10);
			nstep=DEFAULT;
		}
		if (m_stop)
		{
			//结束线程
			nstep=END;
		}
		switch(nstep){
		case OPENPREVIEW:{
			//预览
			bool isOpening=true;
			int nOpenStep=0;
			while(isOpening){
				if (m_stop)
				{
					nOpenStep=4;
				}
				switch(nOpenStep){
				case 0:{
					//生成设备
					//生成解码组件
					//生成渲染组件
					//生成录像模块
					if (createDevice())
					{
						if (NULL!=m_pIVideoDecoder)
						{
							m_pIVideoDecoder->Release();
							m_pIVideoDecoder=NULL;
						}
						pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pIVideoDecoder);
						if (NULL!=m_pIVideoRender)
						{
							m_pIVideoRender->Release();
							m_pIVideoRender=NULL;
						}
						pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pIVideoRender);
						if (NULL!=m_pRecorder)
						{
							m_pRecorder->Release();
							m_pRecorder=NULL;
						}
						pcomCreateInstance(CLSID_Recorder,NULL,IID_IRecorder,(void**)&m_pRecorder);
						if (NULL!=m_pIVideoRender&&NULL!=m_pIVideoDecoder&&NULL!=m_pRecorder)
						{
							//create deviceClient succeed
							nOpenStep=1;
							break;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"create render or decoder or recorder fail";
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"create client fail";
					}
					//create deviceClient fail
					nOpenStep=4;
					break;
					   }
					   break;
				case 1:{
					//注册函数:设备，渲染
					registerCallback(DEVICECLIENT);
					registerCallback(DECODE);
					//初始化：渲染器
					if (0!=m_pIVideoRender->setRenderWnd(m_deviceInfo.m_pWnd))
					{
						nOpenStep=4;
					}else{
						//keep going 
						nOpenStep=2;
					}
					   }
					   break;
				case 2:{
					//连接到设备
					if (connectToDevice())
					{
						nOpenStep=3;
					}else{
						nOpenStep=4;
					}
					   }
					   break;
				case 3:{
					//申请码流
					ipcAutoSwitchStream();
					if (liveSteamRequire())
					{
						nOpenStep=5;
					}else{
						nOpenStep=4;
					}
					   }
					   break;
				case 4:{
					//失败
					isOpening=false;
					m_stop=true;
					   }
					   break;
				case 5:{
					//返回
					isOpening=false;
					   }
					   break;
				}
			}
						 }
						 break;
		case SWITCHSTREAM:{
			//ui切换码流
			if (m_currentStatus==STATUS_CONNECTED)
			{
				IChannelManager *pChannelManger=NULL;
				pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&pChannelManger);
				if (NULL!=pChannelManger)
				{
					QVariantMap channelInfo=pChannelManger->GetChannelInfo(m_deviceInfo.m_uiChannelIdInDataBase);
					m_deviceInfo.m_uiStreamId=channelInfo.value("stream").toInt();
					pChannelManger->Release();
					pChannelManger=NULL;
					if (liveSteamRequire())
					{
						//succeed
					}else{
						//fail
					}
					
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"switchStream fail";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"switchStream fail";
			}
						  }
						  break;
		case SWITCHSTREAMEX:{
			//窗口菜单切换码流
			if (m_currentStatus==STATUS_CONNECTED&&NULL!=m_pdeviceClient)
			{
				if ("IPC"==m_deviceInfo.m_sVendor)
				{
					ISwitchStream *pSwitchStream=NULL;
					m_pdeviceClient->QueryInterface(IID_ISwitchStream,(void**)&pSwitchStream);
					if (NULL!=pSwitchStream)
					{
						if (m_deviceInfo.m_uiStreamId==0)
						{
							pSwitchStream->SwitchStream(1);
							m_deviceInfo.m_uiStreamId=1;
							saveToDataBase();
						}else{
							pSwitchStream->SwitchStream(0);
							m_deviceInfo.m_uiStreamId=0;
							saveToDataBase();
						}
						pSwitchStream->Release();
						pSwitchStream=NULL;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fai as apply ISwitchStream fail";
					}
				}else{
					if (m_deviceInfo.m_uiStreamId==0)
					{
						m_deviceInfo.m_uiStreamId=1;
						liveSteamRequire();
						saveToDataBase();
					}else{
						m_deviceInfo.m_uiStreamId=0;
						liveSteamRequire();
						saveToDataBase();
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail";
			}
							}
							break;
		case IPCAUTOSWITCHSTREAM:{
				//ipc 自动切换码流
			ipcAutoSwitchStream();
								 }
								 break;
		case OPENPTZ:{
			//操作云台
			if (openPTZ())
			{
				//succeed;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"OPENPTZ fail";
			}
					 }
					 break;
		case CLOSEPTZ:{
			//关闭云台
			if (closePTZ())
			{
				//succeed
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"CLOSEPTZ fail";
			}
					  }
					  break;
		case AUTORECONNECT:{
			//自动重连
			int nAutoReConnectStep=0;
			bool bAutoStop=false;
			int nCount=0;
			while(!bAutoStop){
				switch(nAutoReConnectStep){
				case 0:{
					//连接
					m_currentStatus=STATUS_CONNECTING;
					QVariantMap curStatusInfo;
					curStatusInfo.insert("CurrentStatus",m_currentStatus);
					emit sgbackToMainThread(curStatusInfo);
					if (connectToDevice())
					{
						nAutoReConnectStep=1;
					}else{
						nAutoReConnectStep=2;
					}
					   }
					   break;
				case 1:{
					//申请码流
					ipcAutoSwitchStream();
					if (liveSteamRequire())
					{
						nAutoReConnectStep=3;
					}else{
						nAutoReConnectStep=2;
					}
					   }
					   break;
				case 2:{
					//失败
					emit sgstopPreview();
					qDebug()<<__FUNCTION__<<__LINE__<<"autoReConnnect to device fail";
					nAutoReConnectStep=4;
					   }
					   break;
				case 3:{
					//成功
					nAutoReConnectStep=5;
					   }
					   break;
				case 4:{
					//休眠，每隔5s自动重连一次
					nCount++;
					if (nCount<500&&m_stop==false)
					{
						msleep(10);
					}else{
						if (m_stop==true)
						{
							nAutoReConnectStep=5;
						}else{
							nCount=0;
							nAutoReConnectStep=0;
						}
					}
					   }
					   break;
				case 5:{
					//停止自动重连
					bAutoStop=true;
					   }
					   break;
				}
			}
						   }
						   break;
		case STARTRECORD:{
			//开启录像
			if (NULL!=m_pRecorder&&m_bIsRecord==false)
			{
				m_pRecorder->SetDevInfo(m_deviceInfo.m_sDeviceName,m_deviceInfo.m_uiChannelId);
				m_pRecorder->Start();
				m_bIsRecord=true;
			}else{
				if (m_bIsRecord==true)
				{
					// do nothing ,it may had been in recorder
				}else{		
					qDebug()<<__FUNCTION__<<__LINE__<<"STARTRECORD fail";
				}
			}
						 }
						 break;
		case STOPRECORD:{
			//停止录像
			if (NULL!=m_pRecorder&&m_bIsRecord==true&&m_bIsAutoRecording==false)
			{
				m_pRecorder->Stop();
				m_bIsRecord=false;
			}else{
				if (m_bIsRecord==false||m_bIsAutoRecording==true)
				{
					//do nothing
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"STOPRECORD fail";
				}
			}
						}
						break;
		case AUTOSYNTIME:{
			//自动同步时间
			// fix me
			int nSysTimeStep=0;
			bool bSysTimeStop=false;
			while(!bSysTimeStop){
				if (m_stop=true)
				{
					nSysTimeStep=4;
				}
				switch(nSysTimeStep){
				case 0:{
					//获取数据库中设备是否需要自动同步时间
					ILocalSetting *pLocalPlayer=NULL;
					pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void **)&pLocalPlayer);
					if (NULL!=pLocalPlayer)
					{
						m_bIsSysTime=pLocalPlayer->getAutoSyncTime();
						pLocalPlayer->Release();
						pLocalPlayer=NULL;
						nSysTimeStep=1;
					}else{
						nSysTimeStep=2;
					}
					   }
					   break;
				case 1:{
					//自动同步
					if ("IPC"==m_deviceInfo.m_sVendor&&m_bIsSysTime==true&&m_pdeviceClient!=NULL)
					{
						IAutoSycTime *pAutoSysTime=NULL;
						m_pdeviceClient->QueryInterface(IID_IAutoSycTime,(void**)&pAutoSysTime);
						if (NULL!=pAutoSysTime)
						{
							pAutoSysTime->SetAutoSycTime(m_bIsSysTime);
							pAutoSysTime->Release();
							pAutoSysTime=NULL;
							nSysTimeStep=3;
						}else{
							nSysTimeStep=2;
						}
					}else{
						nSysTimeStep=2;
					}
					   }
					   break;
				case 2:{
					//失败
					nSysTimeStep=4;
					   }
					   break;
				case 3:{
					//成功
					nSysTimeStep=4;
					   }
					   break;
				case 4:{
					//end
					bSysTimeStop=true;
					   }
					   break;
				}
			}
						 }
						 break;
		case SETVOLUME:{
			//设置声音大小
			if (NULL!=m_pAudioPlay)
			{
				m_pAudioPlay->SetVolume(m_volumePersent);
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"m_pAudioPlay is null";
			}
					   }
					   break;
		case AUDIOENABLE:{
			//开启声道
			if (m_bIsAudioOpen)
			{
				m_sampleRate=0;
				m_sampleWidth=0;
				if (NULL!=m_pAudioPlay)
				{
					m_pAudioPlay->EnablePlay(true);
					m_pAudioPlay->SetVolume(m_volumePersent);
				}else{
					pcomCreateInstance(CLSID_AudioPlayer,NULL,IID_IAudioPlayer,(void **)&m_pAudioPlay);
					if (NULL!=m_pAudioPlay)
					{
						m_pAudioPlay->EnablePlay(true);
						m_pAudioPlay->SetVolume(m_volumePersent);
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"apply for IAudioPlayer inteface fail";
					}
				}
			}else{
				if (NULL !=m_pAudioPlay)
				{
					m_pAudioPlay->Stop();
					m_pAudioPlay->Release();
					m_pAudioPlay=NULL;
				}
			}
						 }
						 break;
		case DEFAULT:{
			//缺省，无动作
					 }
					 break;
		case END:{
			//END
			//sgstopPreview 阻塞，跳转到另一个线程，避免两个线程同时调用sstopPreview（）；
				emit sgstopPreview();
				nstop=true;
				//释放此函数生成的所有资源
				if (NULL!=m_pIVideoRender)
				{
					m_pIVideoRender->Release();
					m_pIVideoRender=NULL;
				}
				if (NULL!=m_pdeviceClient)
				{
					m_pdeviceClient->Release();
					m_pdeviceClient=NULL;
				}
				if (NULL!=m_pIVideoDecoder)
				{
					m_pIVideoDecoder->Release();
					m_pIVideoDecoder=NULL;
				}
				if (NULL!=m_pRecorder)
				{
					m_pRecorder->Release();
					m_pRecorder=NULL;
				}
				if (NULL!=m_pAudioPlay)
				{
					m_pAudioPlay->Release();
					m_pAudioPlay=NULL;
				}
				//抛出断开的事件
				m_currentStatus=STATUS_DISCONNECTED;
				QVariantMap curStatusInfo;
				curStatusInfo.insert("CurrentStatus",m_currentStatus);
				emit sgbackToMainThread(curStatusInfo);
				 }
				 break;
		}
	}
}

void QSubviewRun::openPreview(int chlId,QWidget *pWnd)
{
	if (QThread::isRunning()||m_currentStatus!=STATUS_DISCONNECTED)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"this preview thread still running,please call stopPreview() function if you want reopen";
		return;
	}else{
		QThread::start();
		m_stepCode.clear();
		m_stepCode.enqueue(OPENPREVIEW);
		m_deviceInfo.m_uiChannelIdInDataBase=chlId;
		m_deviceInfo.m_pWnd=pWnd;
		m_bIsdataBaseFlush=true;
		return;
	}
}

void QSubviewRun::stopPreview()
{
	if (QThread::isRunning())
	{
		//set nstepcode
		emit slstopPreview();
		m_stop=true;
	}else{
		//do nothing
	}
}

void QSubviewRun::switchStream()
{
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		//set nstepcode
		m_stepCode.enqueue(SWITCHSTREAM);
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"switchStream fail";
	}
}

void QSubviewRun::openPTZ( int nCmd,int nSpeed )
{
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		//set nstepcode
		m_stepCode.enqueue(OPENPTZ);
		m_ptzCmd=nCmd;
		m_ptzSpeed=nSpeed;
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"openPTZ fail";
	}
}

bool QSubviewRun::openPTZ()
{
	if (NULL!=m_pdeviceClient)
	{
		IPTZControl *pIpzControl=NULL;
		int ret=-1;
		m_pdeviceClient->QueryInterface(IID_IPTZControl,(void**)&pIpzControl);
		if (NULL!=pIpzControl)
		{
			switch(m_ptzCmd){
			case 0:{
				ret=pIpzControl->ControlPTZUp(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 1:{
				ret=pIpzControl->ControlPTZDown(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 2:{
				ret=pIpzControl->ControlPTZLeft(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 3:{
				ret=pIpzControl->ControlPTZRight(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 4:{
				if (!m_bIsPtzAutoOpen)
				{
					m_bIsPtzAutoOpen=true;
					ret=pIpzControl->ControlPTZAuto(m_deviceInfo.m_uiChannelId,true);
				}else{
					m_bIsPtzAutoOpen=false;
					ret=pIpzControl->ControlPTZAuto(m_deviceInfo.m_uiChannelId,false);
				}
				   }
				   break;
			case 5:{
				ret=pIpzControl->ControlPTZFocusFar(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 6:{
				ret=pIpzControl->ControlPTZFocusNear(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 7:{
				ret=pIpzControl->ControlPTZZoomIn(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 8:{
				ret=pIpzControl->ControlPTZZoomOut(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 9:{
				ret=pIpzControl->ControlPTZIrisOpen(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 10:{
				ret=pIpzControl->ControlPTZIrisClose(m_deviceInfo.m_uiChannelId,m_ptzSpeed);
					}
					break;
			default:
				break;
			}
			if (ret==0)
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"openPTZ fail ,ret =:"<<ret;
				return false;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"openPTZ fail as device client do not support IPTZControl interface";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"openPTZ fail as m_pdeviceClient is null";
	}
	return false;
}

void QSubviewRun::closePTZ( int nCmd )
{
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		//set nstepcode
		m_stepCode.enqueue(CLOSEPTZ);
		m_ptzCmdEx=nCmd;
	}else{
		//do nothing
	}
}

bool QSubviewRun::closePTZ()
{
	if (NULL!=m_pdeviceClient)
	{
		IPTZControl *pPtzControl=NULL;
		m_pdeviceClient->QueryInterface(IID_IPTZControl,(void**)&pPtzControl);
		if (NULL!=pPtzControl)
		{
			if (4!=m_ptzCmdEx)
			{
				int nRet=pPtzControl->ControlPTZStop(m_deviceInfo.m_uiChannelId,m_ptzCmdEx);
				if (nRet==0)
				{
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"closePTZ fail as ret:"<<nRet;
					return false;
				}
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"closePTZ fail as device client do not support IPTZControl interface";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"closePTZ fail as m_pdeviceClient is null";
	}
	return false;
}

void QSubviewRun::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser )
{
	if (!m_eventNameList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event :"<<eventName<<"fail";
		return;
	}else{
		tagProcInfo proInfo;
		proInfo.proc=proc;
		proInfo.puser=pUser;
		m_eventMap.insert(eventName,proInfo);
		return;
	}
}

void QSubviewRun::slstopPreview()
{
	if (QThread::isRunning())
	{
		//断开连接
		IDeviceClient *pdisconnet=NULL;
		if (m_pdeviceClient!=NULL)
		{
			m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdisconnet);
			if (NULL!=pdisconnet)
			{
				pdisconnet->closeAll();
				pdisconnet->Release();
				pdisconnet=NULL;
			}else{
				//do nothing;
				qDebug()<<__FUNCTION__<<__LINE__<<"can not apply for disconnet interface";
			}
		}else{
			// don nothing;
		}
		//停止录像
		if (NULL!=m_pRecorder)
		{
			IRecorder *pRecorder=NULL;
			m_pRecorder->QueryInterface(IID_IRecorder,(void**)&pRecorder);
			if (NULL!=pRecorder)
			{
				pRecorder->Stop();
				pRecorder->Release();
				pRecorder=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"stop recorder can not apply for IRecorder interface";
			}
		}else{

		}
		//
	}else{

	}
}

int QSubviewRun::cbCConnectState( QString evName,QVariantMap evMap,void *pUser )
{
	emit sgbackToMainThread(evMap);
	return 0;
}

void QSubviewRun::eventCallBack( QString eventName,QVariantMap evMap )
{
	if (m_eventNameList.contains(eventName))
	{
		tagProcInfo proInfo=m_eventMap.value(eventName);
		if (NULL!=proInfo.proc)
		{
			proInfo.proc(eventName,evMap,proInfo.puser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<eventName<<" event is not regist";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support :"<<eventName;
	}
}

int QSubviewRun::cbCPreviewData( QString evName,QVariantMap evMap,void *pUuer )
{
	if (NULL!=m_pIVideoDecoder)
	{
		if (m_deviceInfo.m_pWnd->isVisible())
		{
			unsigned int nLength=evMap.value("length").toUInt();
			char * lpdata=(char *)evMap.value("data").toUInt();
			int frameType = evMap.value("frametype").toUInt();
			//音频
			if (NULL!=m_pAudioPlay&&0==frameType&&m_bIsFocus==true)
			{
				int nSampleRate = evMap.value("samplerate").toUInt();
				int nSampleWidth = evMap.value("samplewidth").toUInt();
				if (nSampleRate != m_sampleRate || nSampleWidth != m_sampleWidth)
				{
					m_sampleRate = nSampleRate;
					m_sampleWidth = nSampleWidth;
					m_pAudioPlay->SetAudioParam(1, m_sampleRate, m_sampleWidth);
				}
				m_pAudioPlay->Play(lpdata, nLength);
			}
			//视频解码
			m_pIVideoDecoder->decode(lpdata,nLength);
			return 0;
		}else{
			//do nothing
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_pIVideoDecoder is null";
	}
	return 1;
}


int QSubviewRun::cbCRecorderData( QString evName,QVariantMap evMap,void*pUser )
{
	if (NULL!=m_pRecorder)
	{
		m_pRecorder->InputFrame(evMap);
	}else{
		// do nothing
	}
	return 0;
}

bool QSubviewRun::createDevice()
{
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		QVariantMap channelInfo=pChannelManager->GetChannelInfo(m_deviceInfo.m_uiChannelIdInDataBase);
		m_deviceInfo.m_uiStreamId=channelInfo.value("stream").toInt();
		m_deviceInfo.m_uiChannelId=channelInfo.value("number").toInt();
		m_deviceInfo.m_sCameraname=channelInfo.value("name").toString();
		int dev_id=channelInfo.value("dev_id").toInt();
		pChannelManager->Release();
		pChannelManager=NULL;
		IDeviceManager *pDeviceManager=NULL;
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void **)&pDeviceManager);
		if (NULL!=pDeviceManager)
		{
			QVariantMap deviceInfo=pDeviceManager->GetDeviceInfo(dev_id);
			m_deviceInfo.m_sVendor=deviceInfo.value("vendor").toString();
			m_deviceInfo.m_sPassword=deviceInfo.value("password").toString();
			m_deviceInfo.m_sUsername=deviceInfo.value("username").toString();
			m_deviceInfo.m_sEseeId=deviceInfo.value("eseeid").toString();
			m_deviceInfo.m_sAddress=deviceInfo.value("address").toString();
			m_deviceInfo.m_uiPort=deviceInfo.value("port").toInt();
			m_deviceInfo.m_sDeviceName=deviceInfo.value("name").toString();
			pDeviceManager->Release();
			pDeviceManager=NULL;
			if (m_deviceInfo.m_sVendor.isEmpty()==false)
			{
				QString sAppPath=QCoreApplication::applicationDirPath();
				QFile *file=new QFile(sAppPath+"/pcom_config.xml");
				file->open(QIODevice::ReadOnly);
				QDomDocument ConFile;
				ConFile.setContent(file);
				QDomNode clsidNode=ConFile.elementsByTagName("CLSID").at(0);
				QDomNodeList itemList=clsidNode.childNodes();
				for(int n=0;n<itemList.count();n++){
					QDomNode item=itemList.at(n);
					QString sItemName=item.toElement().attribute("vendor");
					if (sItemName==m_deviceInfo.m_sVendor)
					{
						CLSID DeviceVendorClsid=pcomString2GUID(item.toElement().attribute("clsid"));
						if (m_pdeviceClient!=NULL)
						{
							m_pdeviceClient->Release();
							m_pdeviceClient=NULL;
						}
						pcomCreateInstance(DeviceVendorClsid,NULL,IID_IDeviceClient,(void**)&m_pdeviceClient);
						if (NULL!=m_pdeviceClient)
						{
							//create device suceess;
							file->close();
							delete file;
							file=NULL;
							return true;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"create deviceClient fail";
						}
					}
				}
				file->close();
				delete file;
				file=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"create deviceClient fail because of vendor is empty";
			}

		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"create deviceClient fail because of it can not create CommonLibPlugin or CommonLibPlugin do not supply pDeviceManager interface";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"create deviceClient fail because of it can not create CommonLibPlugin or CommonLibPlugin do not supply pChannelManager interface";
	}
	return false;
}

bool QSubviewRun::registerCallback(int registcode)
{
	IEventRegister *pRegist=NULL;
	switch(registcode){
	case DEVICECLIENT:{
		if (NULL!=m_pdeviceClient)
		{
			m_pdeviceClient->QueryInterface(IID_IEventRegister,(void**)&pRegist);
			if (NULL!=pRegist)
			{
				pRegist->registerEvent(QString("LiveStream"),cbPreviewRData,this);
				pRegist->registerEvent(QString("SocketError"),cbConnectRError,this);
				pRegist->registerEvent(QString("CurrentStatus"),cbConnectRState,this);
				pRegist->registerEvent(QString("ForRecord"),cbRecorderRData,this);
				pRegist->Release();
				pRegist=NULL;
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"deviceClient register fail as pRegist is null";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"register fail as m_pdeviceClient is null";
		}
					  }
					  break;
	case RECORD:{
		//设置录像回调函数
		if (NULL!=m_pRecorder)
		{
			m_pRecorder->QueryInterface(IID_IEventRegister,(void**)&pRegist);
			if (NULL!=pRegist)
			{
				pRegist->registerEvent("RecordState",cbRecordRState,this);
				pRegist->Release();
				pRegist=NULL;
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"recoder register fail as pRegist is null";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"register fail as m_pRecorder is null";
		}
				}
				break;
	case DECODE:{
		//注册解码的回调函数
		if (NULL!=m_pIVideoDecoder)
		{
			m_pIVideoDecoder->QueryInterface(IID_IEventRegister,(void**)&pRegist);
			if (NULL!=pRegist)
			{
				pRegist->registerEvent(QString("DecodedFrame"),cbDecodeRFrame,this);
				pRegist->Release();
				pRegist=NULL;
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"decoder register fail as pRegist is null";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"register fail as m_pIVideoDecoder is null";
		}
				}
				break;
	}
	return false;
}

int QSubviewRun::cbCConnectError( QString evName,QVariantMap evMap,void*pUser )
{
	if (m_currentStatus!=STATUS_DISCONNECTED)
	{
		m_currentStatus=STATUS_DISCONNECTED;
		QVariantMap curStatusInfo;
		curStatusInfo.insert("CurrentStatus",m_currentStatus);
		emit sgbackToMainThread(curStatusInfo);
	}
	return 0;
}
static void YUV420ToRGB888(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height, unsigned char *dst)
{
	int line, col, linewidth;
	int y, u, v, yy, vr, ug, vg, ub;
	int r, g, b;
	unsigned char *pRGB = NULL;

	linewidth = width >> 1;

	y = *py++;
	yy = y << 8;
	u = *pu - 128;
	ug = 88 * u;
	ub = 454 * u;
	v = *pv - 128;
	vg = 183 * v;
	vr = 359 * v;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			r = (yy + vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub ) >> 8;

			if (r < 0) r = 0;
			if (r > 255) r = 255;
			if (g < 0) g = 0;
			if (g > 255) g = 255;
			if (b < 0) b = 0;
			if (b > 255) b = 255;

			pRGB = dst + line*width*3 + col*3;
			*pRGB = r;
			*(pRGB + 1) = g;
			*(pRGB + 2) = b;

			y = *py++;
			yy = y << 8;
			if (col & 1) {
				pu++;
				pv++;

				u = *pu - 128;
				ug = 88 * u;
				ub = 454 * u;
				v = *pv - 128;
				vg = 183 * v;
				vr = 359 * v;
			}
		} 
		if ((line & 1) == 0) { 
			pu -= linewidth;
			pv -= linewidth;
		}
	} 
}

int QSubviewRun::cbCDecodeFrame( QString evName,QVariantMap evMap,void*pUser )
{
	if (NULL!=m_pIVideoRender)
	{
		char* pData=(char*)evMap.value("data").toUInt();	
		char* pYdata=(char*)evMap.value("Ydata").toUInt();
		char* pUdata=(char*)evMap.value("Udata").toUInt();
		char* pVdata=(char*)evMap.value("Vdata").toUInt();
		int iWidth=evMap.value("width").toInt();
		int iHeight=evMap.value("height").toInt();
		int iYStride=evMap.value("YStride").toInt();
		int iUVStride=evMap.value("UVStride").toInt();
		int iLineStride=evMap.value("lineStride").toInt();
		QString iPixeFormat=evMap.value("pixelFormat").toString();
		int iFlags=evMap.value("flags").toInt();

		if (m_bScreenShot)
		{
			m_bScreenShot=false;
			unsigned char *rgbBuff = new unsigned char[iWidth*iHeight*3];
			memset(rgbBuff, 0, iWidth*iHeight*3);
			YUV420ToRGB888((unsigned char*)pYdata, (unsigned char*)pUdata, (unsigned char*)pVdata,iWidth, iHeight, rgbBuff);
			QImage img(rgbBuff, iWidth, iHeight, QImage::Format_RGB888);
			img.save(m_sScreenShotPath, "JPG");
			delete [] rgbBuff;
		}
		if (m_nInitHeight!=iHeight||m_nInitWidth!=iWidth)
		{
			m_pIVideoRender->deinit();
			m_pIVideoRender->init(iWidth,iHeight);
			m_nInitWidth=iWidth;
			m_nInitHeight=iHeight;
		}
		m_pIVideoRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_pIVideoRender is null";
	}
	return 0;
}

int QSubviewRun::cbCRecordState( QString evName,QVariantMap evMap,void*pUser )
{
	eventCallBack(evName,evMap);
	return 0;
}

bool QSubviewRun::connectToDevice()
{
	IDeviceClient *pdeviceClient=NULL;
	m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdeviceClient);
	if (NULL!=pdeviceClient)
	{
		pdeviceClient->checkUser(m_deviceInfo.m_sUsername,m_deviceInfo.m_sPassword);
		pdeviceClient->setChannelName(m_deviceInfo.m_sCameraname);
		pdeviceClient->setDeviceHost(m_deviceInfo.m_sAddress);
		pdeviceClient->setDeviceId(m_deviceInfo.m_sEseeId);
		pdeviceClient->setDevicePorts(m_deviceInfo.m_uiPort);
		if (0==pdeviceClient->connectToDevice())
		{
			int ncount=0;
			while(m_currentStatus==STATUS_CONNECTING&&ncount<500){
				msleep(10);
				ncount++;
			}
			if (m_currentStatus==STATUS_CONNECTED)
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail";
				if (m_currentStatus==STATUS_CONNECTING)
				{
					//
					qDebug()<<__FUNCTION__<<__LINE__<<"there may be some error in deviceClient module";
					m_currentStatus=STATUS_DISCONNECTED;
					QVariantMap curStatusInfo;
					curStatusInfo.insert("CurrentStatus",m_currentStatus);
					emit sgbackToMainThread(curStatusInfo);
				}
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail";
			if (m_currentStatus==STATUS_CONNECTING)
			{
				m_currentStatus=STATUS_DISCONNECTED;
				QVariantMap curStatusInfo;
				curStatusInfo.insert("CurrentStatus",m_currentStatus);
				emit sgbackToMainThread(curStatusInfo);
			}else{
				//do nothing
			}
			
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"apply pdeviceClient fail";
	}
	return false;
}

bool QSubviewRun::liveSteamRequire()
{
	if (NULL!=m_pdeviceClient)
	{
		IDeviceClient *pdeviceClient=NULL;
		m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdeviceClient);
		if (NULL !=pdeviceClient)
		{
			if (pdeviceClient->liveStreamRequire(m_deviceInfo.m_uiChannelId,m_deviceInfo.m_uiStreamId,true)==0)
			{
				pdeviceClient->Release();
				pdeviceClient=NULL;
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail: "<<1;
				pdeviceClient->Release();
				pdeviceClient=NULL;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail as device client do not support IDeviceClient interface";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail as m_pdeviceClient is null";
	}
	return false;
}

void QSubviewRun::ipcAutoSwitchStream()
{
	if (NULL!=m_pdeviceClient)
	{
		ISwitchStream *pSwitchStream=NULL;
		m_pdeviceClient->QueryInterface(IID_ISwitchStream,(void**)&pSwitchStream);
		if (NULL!=pSwitchStream)
		{
			if (m_deviceInfo.m_pWnd->parentWidget()->width()-m_deviceInfo.m_pWnd->width()<20)
			{
				pSwitchStream->SwitchStream(0);
				m_deviceInfo.m_uiStreamId=0;
			}else{
				pSwitchStream->SwitchStream(1);
				m_deviceInfo.m_uiStreamId=1;
			}
			pSwitchStream->Release();
			pSwitchStream=NULL;
		}else{
			//it do not supply ISwitchStream interface
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"ipcAutoSwitchStream fail as m_pdeviceClient is null";
	}
	return;
}

void QSubviewRun::switchStreamEx()
{
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		//set nstepCode
		m_stepCode.enqueue(SWITCHSTREAMEX);
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"switchStreamEx fail";
	}
}

void QSubviewRun::saveToDataBase()
{
	IChannelManager *pChannelManger=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void**)&pChannelManger);
	if (NULL!=pChannelManger)
	{
		pChannelManger->ModifyChannelStream(m_deviceInfo.m_uiChannelIdInDataBase,m_deviceInfo.m_uiStreamId);
		pChannelManger->Release();
		pChannelManger=NULL;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"saveToDataBase fail as pChannelManger is null ";
	}
	return;
}

int QSubviewRun::startRecord()
{
	if (QThread::isRunning())
	{
		if (m_bIsAutoRecording)
		{
			return 2;
		}else{
			if (m_bIsRecord)
			{
				//do nothing 
				qDebug()<<__FUNCTION__<<__LINE__<<"it had been recording";
			}else{
				m_stepCode.enqueue(STARTRECORD);
			}
			return 0;
		}
	}else{
		return 1;
	}
}

int QSubviewRun::stopRecord()
{
	if (QThread::isRunning())
	{
		if (m_bIsAutoRecording)
		{
			return 2;
		}else{
			if (m_bIsRecord)
			{
				m_stepCode.enqueue(STOPRECORD);
			}else{
				//do nothing 
				qDebug()<<__FUNCTION__<<__LINE__<<" it is not in recording";
			}
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<" it is not in recording";
		return 0;
	}
}

void QSubviewRun::slbackToMainThread( QVariantMap evMap )
{
	//连接状态
	if (evMap.contains("CurrentStatus"))
	{
		m_currentStatus=(QSubviewRunConnectStatus)evMap.value("CurrentStatus").toInt();
		if (m_currentStatus==STATUS_CONNECTED)
		{
			//do 
			if (m_historyStatus!=m_currentStatus)
			{
				//开启声音
				m_stepCode.enqueue(AUDIOENABLE);
				//自动同步时间
				m_stepCode.enqueue(AUTOSYNTIME);
				//开启计划录像查询
				m_planRecordTimer.start(1000);
				//抛出事件；
				eventCallBack("CurrentStatus",evMap);
			}
			m_historyStatus=m_currentStatus;
		}else if (m_currentStatus==STATUS_DISCONNECTED)
		{
			//do
			if (m_historyStatus!=m_currentStatus)
			{
				//停止声音

				//停止计划录像查询
				m_planRecordTimer.stop();
				//停止录像
				m_stepCode.enqueue(STOPRECORD);
				//抛出事件
				eventCallBack("CurrentStatus",evMap);
				//自动重连
				if (m_historyStatus==STATUS_CONNECTED)
				{
					m_stepCode.enqueue(AUTORECONNECT);
				}
			}
			m_historyStatus=m_currentStatus;

		}else if (m_currentStatus==STATUS_CONNECTING)
		{
			//do
			//抛出事件
			if (m_historyStatus!=m_currentStatus)
			{
				eventCallBack("CurrentStatus",evMap);
			}
			m_historyStatus=m_currentStatus;

		}else if (m_currentStatus==STATUS_DISCONNECTING)
		{
			//do
			//抛出事件
			if (m_historyStatus!=m_currentStatus)
			{
				eventCallBack("CurrentStatus",evMap);
			}
			m_historyStatus=m_currentStatus;
		}else{
			//do nothing
			qDebug()<<__FUNCTION__<<__LINE__<<"this is a undefined connectState";
		}
	}
	//其他事件也在此处处理
}

void QSubviewRun::slplanRecord()
{
	if (NULL!=m_pRecorder)
	{
		if (m_bIsdataBaseFlush)
		{
			ISetRecordTime *pSetRecordTime=NULL;
			pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ISetRecordTime,(void **)&pSetRecordTime);
			if (NULL!=pSetRecordTime)
			{
				QStringList recordIdList=pSetRecordTime->GetRecordTimeBydevId(m_deviceInfo.m_uiChannelIdInDataBase);
				tagRecorderTimeInfo recTimeInfo;
				for (int i=0;i<recordIdList.size();i++)
				{
					QString recordId=recordIdList[i];
					QVariantMap timeInfo=pSetRecordTime->GetRecordTimeInfo(recordId.toInt());
					recTimeInfo.nEnable = timeInfo.value("enable").toInt();
					recTimeInfo.nWeekDay = timeInfo.value("weekday").toInt();
					int weekDay = QDate::currentDate().dayOfWeek() - 1;

					if (0 == recTimeInfo.nEnable || weekDay != recTimeInfo.nWeekDay)
					{
						continue;
					}
					recTimeInfo.startTime = QTime::fromString(timeInfo.value("starttime").toString().mid(11), "hh:mm:ss");
					recTimeInfo.endTime = QTime::fromString(timeInfo.value("endtime").toString().mid(11), "hh:mm:ss");
					m_lstReocrdTimeInfoList.append(recTimeInfo);
				}
				pSetRecordTime->Release();
				pSetRecordTime=NULL;
			}else{
				m_planRecordTimer.stop();
				qDebug()<<__FUNCTION__<<__LINE__<<"plan record fail as apply for ISetRecordTime interface fail";
			}
		}else{
			//do nothing 
		}
		//keep going
		for (int j=0;j<m_lstReocrdTimeInfoList.size();++j)
		{
			if (0==m_lstReocrdTimeInfoList[j].nEnable||QDate::currentDate().dayOfWeek()-1!=m_lstReocrdTimeInfoList[j].nWeekDay)
			{
				continue;
			}
			QTime currentTime; 
			currentTime=QTime::currentTime();
			if (m_currentStatus==STATUS_CONNECTED&&currentTime>=m_lstReocrdTimeInfoList[j].startTime&&currentTime<m_lstReocrdTimeInfoList[j].endTime&&m_bIsAutoRecording==false)
			{
				m_stepCode.enqueue(STARTRECORD);
				m_bIsAutoRecording=true;
			}
			if (m_bIsAutoRecording==true&&currentTime>=m_lstReocrdTimeInfoList[j].endTime)
			{
				m_stepCode.enqueue(STOPRECORD);
				m_bIsAutoRecording=false;
			}
			if (!m_lstReocrdTimeInfoList.size())
			{
				if (m_bIsAutoRecording==true)
				{
					m_stepCode.enqueue(STOPRECORD);
					m_bIsAutoRecording=false;
				}
			}
		}
	}else{
		//do nothing
		m_planRecordTimer.stop();
	}
}

void QSubviewRun::setDatabaseFlush( bool flag )
{
	m_bIsdataBaseFlush=flag;
}

void QSubviewRun::setVolume( unsigned int uiPersent )
{
	m_volumePersent=uiPersent;
	if (m_bIsAudioOpen==true&&QThread::isRunning())
	{
		m_stepCode.enqueue(SETVOLUME);
	}else{
		//do nothing
	}
}

void QSubviewRun::audioEnabled( bool bEnable )
{
	m_bIsAudioOpen=bEnable;
	if (QThread::isRunning())
	{
		m_stepCode.enqueue(AUDIOENABLE);
	}else{
		//do nothing
	}
}

void QSubviewRun::setFoucs( bool bEnable )
{
	m_bIsFocus=bEnable;
}
QVariantMap QSubviewRun::screenShot()
{
	QVariantMap item;
	m_bScreenShot=true;
	QString sdir=QCoreApplication::applicationDirPath();
	sdir.append("/temp");
	QDir dtemp;
	bool bexist=dtemp.exists(sdir);
	if (bexist==false)
	{
		dtemp.mkdir(sdir);
	}else{
		//keep going
	}
	QDateTime dtime=QDateTime::currentDateTime();
	unsigned int uitime=dtime.toTime_t();
	QString simageName;
	simageName.append(sdir).append("/").append(QString::number(uitime)).append(".jpg");
	m_sScreenShotPath=simageName;
	item.insert("imageName",QString::number(uitime).append(".jpg"));
	item.insert("path",sdir);
	return item;
}

void QSubviewRun::ipcSwitchStream()
{
	if (QThread::isRunning())
	{
		m_stepCode.enqueue(IPCAUTOSWITCHSTREAM);
	}else{
		//do nothing
	}
}

tagDeviceInfo QSubviewRun::deviceInfo()
{
	return m_deviceInfo;
}

int cbConnectRState( QString evName,QVariantMap evMap,void *pUser )
{
	return ((QSubviewRun*)pUser)->cbCConnectState(evName,evMap,pUser);
}

int cbPreviewRData( QString evName,QVariantMap evMap,void *pUser )
{
	return ((QSubviewRun*)pUser)->cbCPreviewData(evName,evMap,pUser);
}


int cbRecorderRData( QString evName,QVariantMap evMap,void*pUser )
{
	return ((QSubviewRun*)pUser)->cbCRecorderData(evName,evMap,pUser);
}
int cbConnectRError( QString evName,QVariantMap evMap,void*pUser )
{
	return ((QSubviewRun*)pUser)->cbCConnectError(evName,evMap,pUser);
}
int cbDecodeRFrame(QString evName,QVariantMap evMap,void*pUser)
{
	return ((QSubviewRun*)pUser)->cbCDecodeFrame(evName,evMap,pUser);
}
int cbRecordRState( QString evName,QVariantMap evMap,void*pUser )
{
	return ((QSubviewRun*)pUser)->cbCRecordState(evName,evMap,pUser);
}