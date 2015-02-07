#include "QSubviewRun.h"
#include <QEventLoop>
#include "IRecorderEx.h"
//#include "vld.h"
#include <QList>
#include "IWindowSettings.h"
#include <IDeviceAuth.h>
int cbConnectRState(QString evName,QVariantMap evMap,void *pUser);
int cbPreviewRData(QString evName,QVariantMap evMap,void *pUser);
int cbRecorderRData(QString evName,QVariantMap evMap,void*pUser);
int cbConnectRError(QString evName,QVariantMap evMap,void*pUser);
int cbDecodeRFrame(QString evName,QVariantMap evMap,void*pUser);
int cbRecordRState(QString evName,QVariantMap evMap,void*pUser);
int cbConnectRefuse(QString evName,QVariantMap evMap,void*pUser);
int cbAuthority(QString evName,QVariantMap evMap,void*pUser);
int cbMotionDetection(QString evName,QVariantMap evMap,void*pUser);
bool QSubviewRun::m_bIsAudioOpen=false;
unsigned int QSubviewRun::m_volumePersent=50;
QList<int > g_tOpenChannelList;
QMutex g_tOpenChannelListLock;
QSubviewRun::QSubviewRun(void):m_pdeviceClient(NULL),
	m_currentStatus(STATUS_DISCONNECTED),
	m_historyStatus(STATUS_DISCONNECTED),
	m_pIVideoDecoder(NULL),
	m_pIVideoRender(NULL),
	m_pRecordDat(NULL),
	m_stop(false),
	m_bIsPtzAutoOpen(false),
	m_bIsSysTime(false),
	m_bIsFocus(false),
	m_bScreenShot(false),
	m_bClosePreview(false),
	m_bIsBlock(false),
	m_bIsSaveRenderFrame(false),
	m_bIsPreRender(false),
	m_bIsPreDecode(false),
	m_bIsManualRecord(false),
	m_nWindId(0),
	m_nRecordType(0),
	m_nPosition(0),
	m_pAudioPlay(NULL),
	m_sampleWidth(0),
	m_sampleRate(0),
	m_nInitHeight(0),
	m_nInitWidth(0),
	m_nSleepSwitch(0),
	m_nCheckPreCount(0),
	m_nSecondPosition(0),
	m_nMotionRecordTime(10),
	m_bStretch(true)
{
	connect(this,SIGNAL(sgbackToMainThread(QVariantMap)),this,SLOT(slbackToMainThread(QVariantMap)));
	connect(this,SIGNAL(sgsetRenderWnd()),this,SLOT(slsetRenderWnd()),Qt::BlockingQueuedConnection);
	connect(&m_tDigitalZoomView,SIGNAL(sgDrawRect(QPoint ,QPoint )),this,SLOT(SLSetRenderRect(QPoint ,QPoint )));
	connect(&m_tDigitalZoomView,SIGNAL(sgHideEvnet()),this,SLOT(slRemoveExtendWnd()));
	connect(&m_tDigitalZoomView,SIGNAL(sgShowEvent()),this,SLOT(slAddExtendWnd()));
	connect(&m_tDigitalZoomView,SIGNAL(sgViewNewPosition(QRect,int ,int )),this,SIGNAL(sgViewNewPosition(QRect,int ,int)));
	m_eventNameList<<"LiveStream"<<"SocketError"<<"CurrentStatus"<<"ForRecord"<<"RecordState"<<"DecodedFrame"<<"ConnectRefuse"<<"Authority"<<"screenShot";
	connect(&m_checkIsBlockTimer,SIGNAL(timeout()),this,SLOT(slcheckoutBlock()));
	m_checkIsBlockTimer.start(4000);
	m_hMainThread=QThread::currentThreadId();
	m_tRenderInfo.pData=NULL;
	m_tRenderInfo.pUdata=NULL;
	m_tRenderInfo.pVdata=NULL;
	m_tRenderInfo.pYdata=NULL;
}


QSubviewRun::~QSubviewRun(void)
{
	if (QThread::isRunning()&&m_stop!=true)
	{
		//set nstep code
		//slstopPreview();
		m_stop=true;
	}else{
		//do nothing
	}
	int n=0;
	while(QThread::isRunning()){
		msleep(10);
		n++;
		if (n>500&&n%100==0)
		{
			sleepEx(10);
			qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<n/100<<"terminate this thread had caused more time than 5s,there may be out of control";
		}
	}
	m_checkIsBlockTimer.stop();
}

void QSubviewRun::run()
{
	//此函数内生成的资源，必须仅在此函数内销毁
	m_stop=false;
	m_bScreenShot=false;
	m_nInitHeight=0;
	m_nInitWidth=0;

	m_tDigitalHisStreamNum=0;
	int nstep=DEFAULT;
	bool nstop=false;
	while(!nstop){
		if (!m_stepCode.isEmpty())
		{
			m_tStepCodeLock.lock();
			nstep=m_stepCode.dequeue();
			m_tStepCodeLock.unlock();
		}else{
			sleepEx(2);
			nstep=DEFAULT;
			/*m_nPosition=__LINE__;*/
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
						m_tVideoRenderLock.lock();
						if (NULL!=m_pIVideoRender)
						{
							m_pIVideoRender->Release();
							m_pIVideoRender=NULL;
						}
						pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pIVideoRender);
						if (NULL != m_pIVideoRender)
						{
							m_pIVideoRender->enableStretch(m_bStretch);
						}
						m_tVideoRenderLock.unlock();
						if (NULL!=m_pRecordDat)
						{
							m_pRecordDat->Release();
							m_pRecordDat=NULL;
						}
						pcomCreateInstance(CLSID_RecordDat,NULL,IID_IRecordDat,(void**)&m_pRecordDat);
						if (NULL!=m_pIVideoRender&&NULL!=m_pIVideoDecoder&&NULL!=m_pRecordDat)
						{
							//create deviceClient succeed
							nOpenStep=1;
							break;
						}else{
							if (NULL==m_pIVideoRender)
							{
								qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<"::"<<m_tDeviceInfo.m_uiChannelId<<"create render fail";
							}else if (NULL==m_pRecordDat)
							{
								qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<"::"<<m_tDeviceInfo.m_uiChannelId<<"create m_pRecordDat fail";
							}
							else{
								qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<"::"<<m_tDeviceInfo.m_uiChannelId<<"create  decoder fail";
							}
										
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<"::"<<m_tDeviceInfo.m_uiChannelId<<"create client fail";
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
					registerCallback(RECORD);
					//初始化：渲染器
					//the fuction of setRenderWnd() can not be call from sub thread
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					emit sgsetRenderWnd();
					m_bIsBlock=false;
					if (0!=/*m_pIVideoRender->setRenderWnd(m_tDeviceInfo.m_pWnd)*/0)
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
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					if (connectToDevice())
					{
						nOpenStep=3;
					}else{
						nOpenStep=4;
					}
					m_bIsBlock=false;
					   }
					   break;
				case 3:{
					//申请码流
					ipcAutoSwitchStream();
					enableStretch();
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					if (liveSteamRequire())
					{
						nOpenStep=5;
					}else{
						nOpenStep=4;
					}
					m_bIsBlock=false;
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
				pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IChannelManager,(void **)&pChannelManger);
				if (NULL!=pChannelManger)
				{
					QVariantMap channelInfo=pChannelManger->GetChannelInfo(m_tDeviceInfo.m_uiChannelIdInDataBase);
					/*m_tDeviceInfo.m_uiStreamId=channelInfo.value("stream").toInt();*/
					if (m_tDeviceInfo.m_uiStreamId==0)
					{
						m_tDeviceInfo.m_uiStreamId=1;
					}else{
						m_tDeviceInfo.m_uiStreamId=0;
					}
					pChannelManger->Release();
					pChannelManger=NULL;
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					if ("IPC"==m_tDeviceInfo.m_sVendor||"ONVIF"==m_tDeviceInfo.m_sVendor)
					{
						ISwitchStream *pSwitchStream=NULL;
						m_pdeviceClient->QueryInterface(IID_ISwitchStream,(void**)&pSwitchStream);
						if (NULL!=pSwitchStream)
						{
							if (m_tDeviceInfo.m_uiStreamId==0)
							{
								pSwitchStream->SwitchStream(0);
								saveToDataBase();
							}else{
								pSwitchStream->SwitchStream(1);
								saveToDataBase();
							}
							pSwitchStream->Release();
							pSwitchStream=NULL;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail as apply ISwitchStream fail";
						}
					}else{
						if (liveSteamRequire())
						{
							//succeed
						}else{
							//fail
						}
					}
					m_bIsBlock=false;
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
				if ("IPC"==m_tDeviceInfo.m_sVendor)
				{
					ISwitchStream *pSwitchStream=NULL;
					m_pdeviceClient->QueryInterface(IID_ISwitchStream,(void**)&pSwitchStream);
					if (NULL!=pSwitchStream)
					{
						if (m_tDeviceInfo.m_uiStreamId==0)
						{
							pSwitchStream->SwitchStream(1);
							m_tDeviceInfo.m_uiStreamId=1;
							saveToDataBase();
						}else{
							pSwitchStream->SwitchStream(0);
							m_tDeviceInfo.m_uiStreamId=0;
							saveToDataBase();
						}
						pSwitchStream->Release();
						pSwitchStream=NULL;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail as apply ISwitchStream fail";
					}
				}else{
					if (m_tDeviceInfo.m_uiStreamId==0)
					{
						m_tDeviceInfo.m_uiStreamId=1;

					}else{
						m_tDeviceInfo.m_uiStreamId=0;
					}
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					liveSteamRequire();
					m_bIsBlock=false;
					saveToDataBase();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail";
			}
							}
							break;
		case DIGITALZOOMTOMAINSTREAM:{
			digitalZoomToMainStream();
									 }
									 break;
		case DIGITALZOOMSTREAMRESTORE:{
			digitalZoomStreamRestore();
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
			int nAutoReConnectStep=4;
			bool bAutoStop=false;
			int nCount=0;
			int nTime=200;
			while(!bAutoStop){
				switch(nAutoReConnectStep){
				case 0:{
					//连接
					if (nTime<800)
					{
						nTime+=100;
					}
					int nCurrentStatus=STATUS_CONNECTING;
					QVariantMap curStatusInfo;
					curStatusInfo.insert("CurrentStatus",nCurrentStatus);
					backToMainThread(curStatusInfo);//emit sgbackToMainThread(curStatusInfo);
					m_nInitHeight=0;
					m_nInitWidth=0;
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					if (connectToDevice())
					{
						nAutoReConnectStep=1;
					}else{
						nAutoReConnectStep=2;
					}
					m_bIsBlock=false;
					   }   
					   break;
				case 1:{
					//申请码流
					ipcAutoSwitchStream();
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					if (liveSteamRequire())
					{
						nAutoReConnectStep=3;
					}else{
						nAutoReConnectStep=2;
					}
					m_bIsBlock=false;
					   }   
					   break;
				case 2:{
					//失败
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					slstopPreview();
					int ncount=0;
					while(m_bClosePreview==true){
						sleepEx(10);
						/*msleep(10);*/
						ncount++;
						if (ncount>500&&ncount%100==0)
						{
							qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<ncount/100<<"run is going terminate,but may be cause crash";
						}
					}
					qDebug()<<__FUNCTION__<<__LINE__<<"autoReConnnect to device fail";
					nAutoReConnectStep=4;
					m_bIsBlock=false;
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
					if (nCount<nTime&&m_stop==false)
					{
						sleepEx(10);
						/*msleep(10);*/
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
		case INITRECORD:{
			//初始化录像
			if (NULL!=m_pRecordDat)
			{
				if (m_pRecordDat->init(m_nWindId))
				{
					m_pRecordDat->updateRecordSchedule(m_tDeviceInfo.m_uiChannelIdInDataBase);
					m_pRecordDat->upDateSystemDatabase();
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"INITRECORD fail as m_pRecordDat->init(m_nWindId) fail";
				}
			}else{
				//do nothing
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as m_pRecordDat is null";
				abort();
			}
						}
						break;
		case DEINITRECORD:{
			//停止录像


			if (NULL!=m_pRecordDat)
			{
				if (m_pRecordDat->deinit())
				{
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"INITRECORD fail as m_pRecordDat->deinit() fail";
				}
			}else{
				//do nothing
			}
						  }
						  break;
		case SETMANUALRECORD:{
			//设置人工录像
			if (NULL!=m_pRecordDat)
			{
				if (m_bIsManualRecord)
				{
					if (m_pRecordDat->manualRecordStart())
					{
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"SETMANUALRECORD fail as m_pRecordDat->manualRecordStart() fail";
					}
				}else{
					if (m_pRecordDat->manualRecordStop())
					{
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"SETMANUALRECORD fail as m_pRecordDat->manualRecordStop() fail";
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SETMANUALRECORD fail as m_pRecordDat is null";
			}
							 }
							 break;
		case SETMOTIONRECORD:{
			//设置移动录像
			if (NULL!=m_pRecordDat)
			{
				m_pRecordDat->motionRecordStart(m_nMotionRecordTime);
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SETMOTIONRECORD fail as m_pRecordDat is null";
			}
							 }
							 break;
		case UPDATEDATABASE:{
			//更新数据库
			if (m_pRecordDat!=NULL)
			{
				m_pRecordDat->updateRecordSchedule(m_tDeviceInfo.m_uiChannelIdInDataBase);
				m_pRecordDat->upDateSystemDatabase();
			}else{
				//do nothing
			}
							}
							break;
		case AUTOSYNTIME:{
			//自动同步时间
			// fix me
			int nSysTimeStep=0;
			bool bSysTimeStop=false;
			while(!bSysTimeStop){
				if (m_stop==true)
				{
					nSysTimeStep=4;
				}
				switch(nSysTimeStep){
				case 0:{
					//获取数据库中设备是否需要自动同步时间
					ILocalSetting *pLocalPlayer=NULL;
					pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_ILocalSetting,(void **)&pLocalPlayer);
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
					if (("IPC"==m_tDeviceInfo.m_sVendor||"ONVIF"==m_tDeviceInfo.m_sVendor)&&m_bIsSysTime==true&&m_pdeviceClient!=NULL)
					{
						IAutoSycTime *pAutoSysTime=NULL;
						m_pdeviceClient->QueryInterface(IID_IAutoSycTime,(void**)&pAutoSysTime);
						if (NULL!=pAutoSysTime)
						{
							m_bIsBlock=true;
							m_nPosition=__LINE__;
							pAutoSysTime->setAutoSycTime(m_bIsSysTime);
							m_bIsBlock=false;
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
						qDebug()<<__FUNCTION__<<__LINE__<<"apply for IAudioPlayer interface fail";
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
		case VEDIOSTRETCH:{
			//保存视频拉伸设置到数据库
			enableStretch();
						  }
						  break;
		case DEFAULT:{
			//缺省，无动作
					 }
					 break;
		case END:{
			//END
			//sgstopPreview 阻塞，跳转到另一个线程，避免两个线程同时调用sstopPreview（）；
			m_bIsBlock=true;
			m_stop=true;
			m_nPosition=__LINE__;
				slstopPreview();
				m_bIsBlock=false;
				int ncount=0;
				while(m_bClosePreview==true){
					sleepEx(10);
					/*msleep(10);*/
					ncount++;
					if (ncount>500&&ncount%100==0)
					{
						qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<"run is going terminate,but may be cause crash";
					}
				}
				nstop=true;
				//释放此函数生成的所有资源
				if (NULL!=m_pdeviceClient)
				{
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					m_pdeviceClient->Release();
					m_bIsBlock=false;
					m_pdeviceClient=NULL;
				}

				if (NULL!=m_pIVideoDecoder)
				{
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					m_pIVideoDecoder->Release();
					m_bIsBlock=false;
					m_pIVideoDecoder=NULL;
				}
				m_tVideoRenderLock.lock();
				if (NULL!=m_pIVideoRender)
				{
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					m_pIVideoRender->Release();
					m_bIsBlock=false;
					m_pIVideoRender=NULL;
				}
				m_tDigitalZoomView.clearRectPoint();
				m_tVideoRenderLock.unlock();
				if (NULL!=m_pRecordDat)
				{
					m_bIsBlock=true;
					m_nPosition=__LINE__;

					m_pRecordDat->deinit();
					m_pRecordDat->Release();
					m_pRecordDat=NULL;
					m_bIsBlock=false;
				}
				if (NULL!=m_pAudioPlay)
				{
					m_bIsBlock=true;
					m_nPosition=__LINE__;
					m_pAudioPlay->Release();
					m_bIsBlock=false;
					m_pAudioPlay=NULL;
				}
				if (NULL!=m_tRenderInfo.pData)
				{
					delete m_tRenderInfo.pData;
					m_tRenderInfo.pData=NULL;
				}else{

				}
				if (NULL!=m_tRenderInfo.pUdata)
				{
					delete m_tRenderInfo.pUdata;
					m_tRenderInfo.pUdata=NULL;
				}else{

				}
				if (NULL!=m_tRenderInfo.pVdata)
				{
					delete m_tRenderInfo.pVdata;
					m_tRenderInfo.pVdata=NULL;
				}else{

				}
				if (NULL!=m_tRenderInfo.pYdata)
				{
					delete m_tRenderInfo.pYdata;
					m_tRenderInfo.pYdata=NULL;
				}else{

				}
				m_bIsSaveRenderFrame=false;
				//抛出断开的事件
				int nCurrentStatus=STATUS_DISCONNECTED;
				QVariantMap curStatusInfo;
				curStatusInfo.insert("CurrentStatus",nCurrentStatus);
				m_bIsBlock=true;
				m_nPosition=__LINE__;
				backToMainThread(curStatusInfo);//emit sgbackToMainThread(curStatusInfo);
				m_bIsBlock=false;
				g_tOpenChannelListLock.lock();
				if (g_tOpenChannelList.contains(m_tDeviceInfo.m_uiChannelIdInDataBase))
				{
					g_tOpenChannelList.removeOne(m_tDeviceInfo.m_uiChannelIdInDataBase);
				}else{
					//do nothing
				}
				g_tOpenChannelListLock.unlock();
				 }
				 break;
		}
	}
	return;
}

void QSubviewRun::openPreview(int chlId,QWidget *pWnd,QWidget *pMainWnd)
{
	if (QThread::isRunning()||m_currentStatus!=STATUS_DISCONNECTED)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"this preview thread still running,please call stopPreview() function if you want reopen";
		return;
	}else{
		g_tOpenChannelListLock.lock();
		if (g_tOpenChannelList.contains(chlId))
		{
			g_tOpenChannelListLock.unlock();
			return;
		}
		g_tOpenChannelListLock.unlock();
		int nCurrentStatus=STATUS_CONNECTING;
		QVariantMap curStatusInfo;
		curStatusInfo.insert("CurrentStatus",nCurrentStatus);
		backToMainThread(curStatusInfo);
		m_tDeviceInfo.m_uiChannelIdInDataBase=chlId;
		m_tDeviceInfo.m_pWnd=pWnd;
		m_tDeviceInfo.m_pMainWnd=pMainWnd;
		m_tStepCodeLock.lock();
		m_stepCode.clear();
		m_stepCode.enqueue(OPENPREVIEW);
		m_tStepCodeLock.unlock();
		g_tOpenChannelListLock.lock();
		g_tOpenChannelList.append(chlId);
		g_tOpenChannelListLock.unlock();
		QThread::start();
		return;
	}
}

void QSubviewRun::stopPreview()
{
	g_tOpenChannelListLock.lock();
	if (g_tOpenChannelList.contains(m_tDeviceInfo.m_uiChannelIdInDataBase))
	{
		g_tOpenChannelList.removeOne(m_tDeviceInfo.m_uiChannelIdInDataBase);
	}else{
		//do nothing
	}
	g_tOpenChannelListLock.unlock();
	if (QThread::isRunning()&&m_stop!=true)
	{
		//set nstepcode
		slstopPreview();
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
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(SWITCHSTREAM);
		m_tStepCodeLock.unlock();
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
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(OPENPTZ);
		m_tStepCodeLock.unlock();
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
				ret=pIpzControl->ControlPTZUp(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 1:{
				ret=pIpzControl->ControlPTZDown(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 2:{
				ret=pIpzControl->ControlPTZLeft(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 3:{
				ret=pIpzControl->ControlPTZRight(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 4:{
				if (!m_bIsPtzAutoOpen)
				{
					m_bIsPtzAutoOpen=true;
					ret=pIpzControl->ControlPTZAuto(m_tDeviceInfo.m_uiChannelId,true);
				}else{
					m_bIsPtzAutoOpen=false;
					ret=pIpzControl->ControlPTZAuto(m_tDeviceInfo.m_uiChannelId,false);
				}
				   }
				   break;
			case 5:{
				ret=pIpzControl->ControlPTZFocusFar(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 6:{
				ret=pIpzControl->ControlPTZFocusNear(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 7:{
				ret=pIpzControl->ControlPTZZoomIn(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 8:{
				ret=pIpzControl->ControlPTZZoomOut(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 9:{
				ret=pIpzControl->ControlPTZIrisOpen(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
				   }
				   break;
			case 10:{
				ret=pIpzControl->ControlPTZIrisClose(m_tDeviceInfo.m_uiChannelId,m_ptzSpeed);
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
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(CLOSEPTZ);
		m_tStepCodeLock.unlock();
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
				int nRet=pPtzControl->ControlPTZStop(m_tDeviceInfo.m_uiChannelId,m_ptzCmdEx);
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
	if (m_bClosePreview!=true)
	{
		m_bClosePreview=true;
		QFuture<void>ret=QtConcurrent::run(this,&QSubviewRun::slstopPreviewrun);
	}else{
		//do nothing
	}

}

int QSubviewRun::cbCConnectState( QString evName,QVariantMap evMap,void *pUser )
{
	backToMainThread(evMap);//emit sgbackToMainThread(evMap);
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
	int nDecodeStep=0;
	int bDecodeStop=false;
	while(bDecodeStop==false&&m_stop==false){
		switch(nDecodeStep){
		case 0:{
			//解码指针是否为空
			if (NULL!=m_pIVideoDecoder)
			{
				nDecodeStep=1;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"decode fail as the m_pIVideoDecoder is null";
				nDecodeStep=3;
			}
			   }
			   break;
		case 1:{
			//窗口是否可视
			if (m_bIsSaveRenderFrame==false)
			{
				nDecodeStep=2;
			}else{
				if (m_tDeviceInfo.m_pWnd->isVisible())
				{
					nDecodeStep=2;
					int nFrameType=evMap.value("frametype").toUInt();
					if (nFrameType==0x01)
					{
						m_bIsSaveRenderFrame=false;
						m_nInitHeight=0;
						m_nInitWidth=0;
					}else{
						//do nothing
					}
				}else{
					nDecodeStep=3;
				}
			}
			   }
			   break;
		case 2:{
			//解码
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
			if (frameType==1||frameType==2)
			{
				m_bIsPreDecode=true;
				if (m_bIsSaveRenderFrame==true)
				{
					//do nothing
					renderSaveFrame();
				}else{
					m_pIVideoDecoder->decode(lpdata,nLength);
				}
			}else{
				//do nothing
			}

			nDecodeStep=4;
			   }
			   break;
		case 3:{
			//不解码
			nDecodeStep=4;
			   }
			   break;
		case 4:{
			//返回
			bDecodeStop=true;
			   }
			   break;
		}
	}
	return 1;
}


int QSubviewRun::cbCRecorderData( QString evName,QVariantMap evMap,void*pUser )
{
	if (NULL!=m_pRecordDat&&m_stop==false)
	{
		m_pRecordDat->inputFrame(evMap);
	}else{
		//do nothing
	}
	return 0;
}

bool QSubviewRun::createDevice()
{
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		QVariantMap channelInfo=pChannelManager->GetChannelInfo(m_tDeviceInfo.m_uiChannelIdInDataBase);
		m_tDeviceInfo.m_uiStreamId=channelInfo.value("stream").toInt();
		m_tDeviceInfo.m_uiChannelId=channelInfo.value("number").toInt();
		m_tDeviceInfo.m_sCameraname=channelInfo.value("name").toString();
		int dev_id=channelInfo.value("dev_id").toInt();
		pChannelManager->Release();
		pChannelManager=NULL;
		IDeviceManager *pDeviceManager=NULL;
		pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDeviceManager,(void **)&pDeviceManager);
		if (NULL!=pDeviceManager)
		{
			QVariantMap deviceInfo=pDeviceManager->GetDeviceInfo(dev_id);
			m_tDeviceInfo.m_sVendor=deviceInfo.value("vendor").toString();
			m_tDeviceInfo.m_sPassword=deviceInfo.value("password").toString();
			m_tDeviceInfo.m_sUsername=deviceInfo.value("username").toString();
			m_tDeviceInfo.m_sEseeId=deviceInfo.value("eseeid").toString();
			m_tDeviceInfo.m_sAddress=deviceInfo.value("address").toString();
			m_tDeviceInfo.m_uiPort=deviceInfo.value("port").toInt();
			m_tDeviceInfo.m_sDeviceName=deviceInfo.value("name").toString();
			m_tDeviceInfo.m_sConnectMethod=deviceInfo.value("method").toString();
			pDeviceManager->Release();
			pDeviceManager=NULL;
			if (m_tDeviceInfo.m_sVendor.isEmpty()==false)
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
					if (sItemName==m_tDeviceInfo.m_sVendor)
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
				qDebug()<<__FUNCTION__<<__LINE__<<"create deviceClient fail because of it can not deviceClent defined in xml";
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
				pRegist->registerEvent(QString("ConnectRefuse"),cbConnectRefuse,this);
				pRegist->registerEvent(QString("Authority"),cbAuthority,this);
				pRegist->registerEvent(QString("MDSignal"),cbMotionDetection,this);
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
		if (NULL!=m_pRecordDat)
		{
			m_pRecordDat->QueryInterface(IID_IEventRegister,(void**)&pRegist);
			if (NULL!=pRegist)
			{
				pRegist->registerEvent("RecordState",cbRecordRState,this);
				pRegist->Release();
				pRegist=NULL;
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"recorder register fail as pRegist is null";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"recorder register fail as m_pRecordDat is null";
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
		int nCurrentStatus=STATUS_DISCONNECTED;
		QVariantMap curStatusInfo;
		curStatusInfo.insert("CurrentStatus",nCurrentStatus);
		m_nSecondPosition=__LINE__;
		backToMainThread(curStatusInfo);//emit sgbackToMainThread(curStatusInfo);
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
int QSubviewRun::cbCDecodeFrame(QString evName,QVariantMap evMap,void*pUser){
	if (m_currentStatus!=STATUS_CONNECTED)
	{
		return 0;
	}
	int nRenderStep=0;
	bool bRenderStop=false;
	m_tVideoRenderLock.lock();
	while(bRenderStop==false){
		switch(nRenderStep){
		case 0:{
			//判断渲染指针
			if (NULL!=m_pIVideoRender)
			{
				nRenderStep=1;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"render fail as the m_pIVideoRender is null";
				nRenderStep=6;
			}
			   }
			   break;
		case 1:{
			//判断是否可视
			if (!m_tDeviceInfo.m_pWnd->isVisible()&&m_bIsSaveRenderFrame==false)
			{
				//save frame
				if (m_tRenderInfo.pData!=NULL)
				{
					delete m_tRenderInfo.pData;
					m_tRenderInfo.pData=NULL;
				}else{
					//do nothing
				}
				if (m_tRenderInfo.pUdata!=NULL)
				{
					delete m_tRenderInfo.pUdata;
					m_tRenderInfo.pUdata=NULL;
				}else{
					//do nothing
				}
				if (m_tRenderInfo.pVdata!=NULL)
				{
					delete m_tRenderInfo.pVdata;
					m_tRenderInfo.pVdata=NULL;
				}else{
					//do nothing
				}
				if (m_tRenderInfo.pYdata!=NULL)
				{
					delete m_tRenderInfo.pYdata;
					m_tRenderInfo.pYdata=NULL;
				}else{
					//do nothing
				}
				m_tRenderInfo.nWidth=evMap.value("width").toInt();
				m_tRenderInfo.nHeight=evMap.value("height").toInt();
				m_tRenderInfo.nYStride=evMap.value("YStride").toInt();
				m_tRenderInfo.nUVStride=evMap.value("UVStride").toInt();
				m_tRenderInfo.nLineStride=evMap.value("lineStride").toInt();
				m_tRenderInfo.sPixeFormat=evMap.value("pixelFormat").toString();
				m_tRenderInfo.nFlags=evMap.value("flags").toInt();


				m_tRenderInfo.pUdata=new char[m_tRenderInfo.nHeight*m_tRenderInfo.nUVStride/2];
				memset(m_tRenderInfo.pUdata, 0, m_tRenderInfo.nHeight*m_tRenderInfo.nUVStride/2);
				memcpy(m_tRenderInfo.pUdata,(char*)evMap.value("Udata").toUInt(),m_tRenderInfo.nHeight*m_tRenderInfo.nUVStride/2);

				m_tRenderInfo.pVdata=new char[m_tRenderInfo.nHeight*m_tRenderInfo.nUVStride/2];
				memset(m_tRenderInfo.pVdata, 0, m_tRenderInfo.nHeight*m_tRenderInfo.nUVStride/2);
				memcpy(m_tRenderInfo.pVdata,(char*)evMap.value("Vdata").toUInt(),m_tRenderInfo.nHeight*m_tRenderInfo.nUVStride/2);

				m_tRenderInfo.pYdata=new char[m_tRenderInfo.nHeight*m_tRenderInfo.nYStride];
				memset(m_tRenderInfo.pYdata, 0, m_tRenderInfo.nHeight*m_tRenderInfo.nYStride);
				memcpy(m_tRenderInfo.pYdata,(char*)evMap.value("Ydata").toUInt(),m_tRenderInfo.nHeight*m_tRenderInfo.nYStride);
				m_bIsSaveRenderFrame=true;
				m_nInitHeight=0;
				m_nInitWidth=0;
				nRenderStep=2;
			}else {
				if (m_tDeviceInfo.m_pWnd->isVisible())
				{
					if (m_bIsSaveRenderFrame==true)
					{
						nRenderStep=2;
					}else{
						nRenderStep=3;
					}
				}else{
					nRenderStep=4;
				}
			
			}
			   }
			   break;
		case 2:{
			//渲染历史帧
			m_bIsPreRender=true;
			nRenderStep=4;
			if (m_nInitHeight!=m_tRenderInfo.nHeight||m_nInitWidth!=m_tRenderInfo.nWidth)
			{
				m_nSecondPosition=__LINE__;
				m_pIVideoRender->deinit();
				m_pIVideoRender->init(m_tRenderInfo.nWidth,m_tRenderInfo.nHeight);
				m_nInitWidth=m_tRenderInfo.nWidth;
				m_nInitHeight=m_tRenderInfo.nHeight;
			}
			m_nSecondPosition=__LINE__;
			m_pIVideoRender->render(m_tRenderInfo.pData,m_tRenderInfo.pYdata,m_tRenderInfo.pUdata,m_tRenderInfo.pVdata,m_tRenderInfo.nWidth,m_tRenderInfo.nHeight,m_tRenderInfo.nYStride,m_tRenderInfo.nUVStride,m_tRenderInfo.nLineStride,m_tRenderInfo.sPixeFormat,m_tRenderInfo.nFlags);
			//截屏
			if (m_bScreenShot)
			{
				QString sFileName;
				QString sFileDir;
				quint64 uiTime;
				int nType;
				int nChl;
				m_bScreenShot=false;
				if (getScreenShotInfo(sFileName,sFileDir,uiTime,nChl,nType))
				{
					unsigned char *rgbBuff = new unsigned char[m_tRenderInfo.nWidth*m_tRenderInfo.nHeight*3];
					memset(rgbBuff, 0, m_tRenderInfo.nWidth*m_tRenderInfo.nHeight*3);
					YUV420ToRGB888((unsigned char*)m_tRenderInfo.pYdata, (unsigned char*)m_tRenderInfo.pUdata, (unsigned char*)m_tRenderInfo.pVdata,m_tRenderInfo.nWidth, m_tRenderInfo.nHeight, rgbBuff);
					QImage img(rgbBuff, m_tRenderInfo.nWidth, m_tRenderInfo.nHeight, QImage::Format_RGB888);
					QString sFilePath=sFileDir+"/"+sFileName;
					img.save(sFilePath, "JPG");
					delete [] rgbBuff;
					if (saveScreenShotInfoToDatabase(sFileName,sFileDir,uiTime,nChl,nType))
					{
						QVariantMap tScreenShotInfo;
						tScreenShotInfo.insert("fileName",sFileName);
						tScreenShotInfo.insert("fileDir",sFileDir);
						tScreenShotInfo.insert("chl",nChl);
						tScreenShotInfo.insert("type",nType);
						tScreenShotInfo.insert("user",m_sScreenUser);
						eventCallBack("screenShot",tScreenShotInfo);
						//do nothing
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"save screenShot info to database fail as saveScreenShotInfoToDatabase";
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as get getScreenShotInfo fail";
					//do nothing
				}
			}
			   }
			   break;
		case 3:{
			//渲染当前帧
			m_bIsPreRender=true;
			nRenderStep=4;
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
			if (m_nInitHeight!=iHeight||m_nInitWidth!=iWidth)
			{
				m_nSecondPosition=__LINE__;
				m_pIVideoRender->deinit();
				m_pIVideoRender->init(iWidth,iHeight);
				m_nInitWidth=iWidth;
				m_nInitHeight=iHeight;
			}
			m_nSecondPosition=__LINE__;
			m_pIVideoRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);
			//截屏
			if (m_bScreenShot)
			{
				QString sFileName;
				QString sFileDir;
				quint64 uiTime;
				int nType;
				int nChl;
				m_bScreenShot=false;
				if (getScreenShotInfo(sFileName,sFileDir,uiTime,nChl,nType))
				{
					unsigned char *rgbBuff = new unsigned char[iWidth*iHeight*3];
					memset(rgbBuff, 0, iWidth*iHeight*3);
					YUV420ToRGB888((unsigned char*)pYdata, (unsigned char*)pUdata, (unsigned char*)pVdata,iWidth, iHeight, rgbBuff);
					QImage img(rgbBuff, iWidth, iHeight, QImage::Format_RGB888);
					QString sFilePath=sFileDir+"/"+sFileName;
					img.save(sFilePath, "JPG");
					delete [] rgbBuff;
					if (saveScreenShotInfoToDatabase(sFileName,sFileDir,uiTime,nChl,nType))
					{
						QVariantMap tScreenShotInfo;
						tScreenShotInfo.insert("fileName",sFileName);
						tScreenShotInfo.insert("fileDir",sFileDir);
						tScreenShotInfo.insert("chl",nChl);
						tScreenShotInfo.insert("type",nType);
						tScreenShotInfo.insert("user",m_sScreenUser);
						eventCallBack("screenShot",tScreenShotInfo);
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"save screenShot info to database fail as saveScreenShotInfoToDatabase";
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as get getScreenShotInfo fail";
					//do nothing
				}
			}
			   }
			   break;
		case 4:{
			//do nothing

			nRenderStep=5;
			   }
				break;
		case 5:{
			//成功
			nRenderStep=7;
			   }
			   break;
		case 6:{
			//失败
			nRenderStep=7;
			   }
			   break;
		case 7:{
			//结束
			bRenderStop=true;
			   }
			   break;
		}
	}
	m_tVideoRenderLock.unlock();
	return 0;
}


int QSubviewRun::cbCRecordState( QString evName,QVariantMap evMap,void*pUser )
{
	eventCallBack(evName,evMap);
	return 0;
}

bool QSubviewRun::connectToDevice()
{
	// 设置认证信息
	IDeviceAuth * pDa = NULL;
	m_pdeviceClient->QueryInterface(IID_IDeviceAuth,(void **)&pDa);
	if (NULL != pDa)
	{
		pDa->setDeviceAuth(m_tDeviceInfo.m_sUsername,m_tDeviceInfo.m_sPassword);
		pDa->Release();
		pDa = NULL;
	}

	IDeviceClient *pdeviceClient=NULL;
	m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdeviceClient);
	if (NULL!=pdeviceClient)
	{
		pdeviceClient->checkUser(m_tDeviceInfo.m_sUsername,m_tDeviceInfo.m_sPassword);
		pdeviceClient->setChannelName(m_tDeviceInfo.m_sCameraname);
		if (m_tDeviceInfo.m_sConnectMethod=="0")
		{
			pdeviceClient->setDeviceHost(m_tDeviceInfo.m_sAddress);
			pdeviceClient->setDeviceId("0");
		}else{
			pdeviceClient->setDeviceHost("");
			pdeviceClient->setDeviceId(m_tDeviceInfo.m_sEseeId);
		}

		pdeviceClient->setDevicePorts(m_tDeviceInfo.m_uiPort);
		m_nSecondPosition=__LINE__;
		if (0==pdeviceClient->connectToDevice())
		{
			int ncount=0;
			while(m_currentStatus==STATUS_CONNECTING&&ncount<500){
				msleep(10);
				ncount++;
			}
			if (m_currentStatus==STATUS_CONNECTED)
			{
				pdeviceClient->Release();
				pdeviceClient=NULL;
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail"<<QThread::currentThreadId();
				if (m_currentStatus==STATUS_CONNECTING)
				{
					//
					qDebug()<<__FUNCTION__<<__LINE__<<"there may be some error in deviceClient module";
					int nCurrentStatus=STATUS_DISCONNECTED;
					QVariantMap curStatusInfo;
					curStatusInfo.insert("CurrentStatus",nCurrentStatus);
					m_nSecondPosition=__LINE__;
					backToMainThread(curStatusInfo);//emit sgbackToMainThread(curStatusInfo);
				}
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail";
			if (m_currentStatus==STATUS_CONNECTING)
			{
				int nCurrentStatus=STATUS_DISCONNECTED;
				QVariantMap curStatusInfo;
				curStatusInfo.insert("CurrentStatus",nCurrentStatus);
				backToMainThread(curStatusInfo);//emit sgbackToMainThread(curStatusInfo);
			}else{
				//do nothing
			}
			
		}
		pdeviceClient->Release();
		pdeviceClient=NULL;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"apply pdeviceClient fail";
	}
	m_nSecondPosition=__LINE__;
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
			m_nSecondPosition=__LINE__;
			if (pdeviceClient->liveStreamRequire(m_tDeviceInfo.m_uiChannelId,m_tDeviceInfo.m_uiStreamId,true)==0)
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
			if (m_tDeviceInfo.m_pMainWnd->width()-m_tDeviceInfo.m_pWnd->width()<20)
			{
				pSwitchStream->SwitchStream(0);
				m_tDeviceInfo.m_uiStreamId=0;
			}else{
				pSwitchStream->SwitchStream(1);
				m_tDeviceInfo.m_uiStreamId=1;
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
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(SWITCHSTREAMEX);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"switchStreamEx fail";
	}
}

void QSubviewRun::saveToDataBase()
{
	IChannelManager *pChannelManger=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IChannelManager,(void**)&pChannelManger);
	if (NULL!=pChannelManger)
	{
		pChannelManger->ModifyChannelStream(m_tDeviceInfo.m_uiChannelIdInDataBase,m_tDeviceInfo.m_uiStreamId);
		pChannelManger->Release();
		pChannelManger=NULL;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"saveToDataBase fail as pChannelManger is null ";
	}
	return;
}


void QSubviewRun::slbackToMainThread( QVariantMap evMap )
{
	//连接状态
	int chlid=m_tDeviceInfo.m_uiChannelIdInDataBase;
	if (evMap.contains("CurrentStatus"))
	{
		m_currentStatus=(QSubviewRunConnectStatus)evMap.value("CurrentStatus").toInt();
		if (m_currentStatus==STATUS_CONNECTED)
		{
			//do 
			if (m_historyStatus!=m_currentStatus)
			{
				//开启声音
				m_tStepCodeLock.lock();
				m_stepCode.enqueue(AUDIOENABLE);
				//自动同步时间
				m_stepCode.enqueue(AUTOSYNTIME);
				//init录像
				m_stepCode.enqueue(INITRECORD);
				if (m_bIsManualRecord)
				{
					m_stepCode.enqueue(SETMANUALRECORD);
				}else{
					//do nothing
				}
				m_tStepCodeLock.unlock();
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

				//停止录像
				m_tStepCodeLock.lock();
				m_stepCode.enqueue(DEINITRECORD);
				m_tStepCodeLock.unlock();
				//抛出事件
				m_nSecondPosition=__LINE__;
				eventCallBack("CurrentStatus",evMap);
				//自动重连
				if (m_historyStatus==STATUS_CONNECTED)
				{
					m_tStepCodeLock.lock();
					m_stepCode.enqueue(AUTORECONNECT);
					m_tStepCodeLock.unlock();
				}
			}
			m_historyStatus=m_currentStatus;

		}else if (m_currentStatus==STATUS_CONNECTING)
		{
			//do
			//抛出事件
			if (m_historyStatus!=m_currentStatus)
			{
				m_nSecondPosition=__LINE__;
				eventCallBack("CurrentStatus",evMap);
			}
			m_historyStatus=m_currentStatus;

		}else if (m_currentStatus==STATUS_DISCONNECTING)
		{
			//do
			//抛出事件
			if (m_historyStatus!=m_currentStatus)
			{
				m_nSecondPosition=__LINE__;
				eventCallBack("CurrentStatus",evMap);
			}
			m_historyStatus=m_currentStatus;
		}else{
			//do nothing
			qDebug()<<__FUNCTION__<<__LINE__<<"this is a undefined connectState";
		}
	}
	//其他事件也在此处处理
	if (evMap.contains("eventName")&&evMap.value("eventName")=="setRenderWnd")
	{
		m_nSecondPosition=__LINE__;
		m_pIVideoRender->setRenderWnd(m_tDeviceInfo.m_pWnd);
	}
}

void QSubviewRun::setDatabaseFlush( bool flag )
{
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(UPDATEDATABASE);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

void QSubviewRun::setVolume( unsigned int uiPersent )
{
	m_volumePersent=uiPersent;
	if (m_bIsAudioOpen==true&&QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(SETVOLUME);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

void QSubviewRun::audioEnabled( bool bEnable )
{
	m_bIsAudioOpen=bEnable;
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(AUDIOENABLE);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

void QSubviewRun::setFoucs( bool bEnable )
{
	m_bIsFocus=bEnable;
}
void QSubviewRun::screenShot(QString sUser,int nType,int nChl)
{
	if (nChl<0||nType!=0)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"the system call abort as the input parm is error";
		abort();
	}else{
		//keep going
	}
	if (QThread::isRunning())
	{
		//设置 截屏开启条件
		m_nScreenShotChl=nChl;
		m_nScreenShotType=nType;
		m_sScreenUser=sUser;
		m_bScreenShot=true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as the thread is not running";
		//do nothing
	}
	return ;
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
	return ;
}

void QSubviewRun::ipcSwitchStream()
{
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(IPCAUTOSWITCHSTREAM);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

tagDeviceInfo QSubviewRun::deviceInfo()
{
	return m_tDeviceInfo;
}

void QSubviewRun::slsetRenderWnd()
{
	m_pIVideoRender->setRenderWnd(m_tDeviceInfo.m_pWnd);
}

void QSubviewRun::slstopPreviewrun()
{
	m_bClosePreview=true;
	if (QThread::isRunning())
	{
		//断开连接
		IDeviceClient *pdisconnet=NULL;
		if (m_pdeviceClient!=NULL)
		{
			m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdisconnet);
			if (NULL!=pdisconnet)
			{
				m_nSecondPosition=__LINE__;
				pdisconnet->closeAll();
				pdisconnet->Release();
				pdisconnet=NULL;
			}else{
				//do nothing;
				qDebug()<<__FUNCTION__<<__LINE__<<"can not apply for disconnect interface";
			}
		}else{
			// do nothing;
		}
	}else{

	}
	m_bClosePreview=false;
}

void QSubviewRun::slcheckoutBlock()
{
	if (m_bIsBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<"block at:"<<m_nPosition<<m_nSecondPosition;
	}
	if (m_nCheckPreCount>12&&m_currentStatus==STATUS_CONNECTED&&m_tDeviceInfo.m_pWnd->isVisible())
	{
		m_nCheckPreCount=0;
		if (m_bIsPreDecode==false)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<m_tDeviceInfo.m_uiChannelId<<"it had continue 60s without preStream,please check";
		}else{
			//do nothing
		}
		if (m_bIsPreDecode==true&&m_bIsPreRender==false)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<m_tDeviceInfo.m_uiChannelId<<"it had continue 60s without preRenderData,please check";
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
	//test 移动侦测
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		/*m_stepCode.enqueue(SETMOTIONRECORD);*/
	}else{
		//do nothing
	}
	m_bIsPreRender=false;
	m_bIsPreDecode=false;
	m_nCheckPreCount++;
}

void QSubviewRun::backToMainThread( QVariantMap evMap )
{
	if (currentThreadId()==m_hMainThread)
	{
		slbackToMainThread(evMap);
	}else{
		emit sgbackToMainThread(evMap);
	}
}

void QSubviewRun::sleepEx( int time )
{
	if (m_nSleepSwitch<5)
	{
		msleep(time);
		m_nSleepSwitch++;
	}else{
		QEventLoop eventloop;
		QTimer::singleShot(100, &eventloop, SLOT(quit()));
		eventloop.exec();
		m_nSleepSwitch=0;
	}
	return;
}


void QSubviewRun::setWindId( int nWindId )
{
	if (nWindId < 0)
	{
		return;
	}
	m_nWindId = nWindId;
}

int QSubviewRun::cbCConnectRefuse( QString evName,QVariantMap evMap,void*pUser )
{
	if (evMap.contains("ConnectRefuse"))
	{
		m_stop=true;
		m_nSecondPosition=__LINE__;
		eventCallBack("ConnectRefuse",evMap);
		qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sAddress<<"Connect to device fail as the devcieClient resource had been full load";
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"undefined callBack event,please checkout";
	}
	return 0;
}

void QSubviewRun::renderSaveFrame()
{
	m_tVideoRenderLock.lock();
	if (NULL!=m_pIVideoRender)
	{
		if (m_nInitHeight!=m_tRenderInfo.nHeight||m_nInitWidth!=m_tRenderInfo.nWidth)
		{
			m_pIVideoRender->deinit();
			m_pIVideoRender->init(m_tRenderInfo.nWidth,m_tRenderInfo.nHeight);
			m_nInitWidth=m_tRenderInfo.nWidth;
			m_nInitHeight=m_tRenderInfo.nHeight;
		}
		m_pIVideoRender->render(m_tRenderInfo.pData,m_tRenderInfo.pYdata,m_tRenderInfo.pUdata,m_tRenderInfo.pVdata,m_tRenderInfo.nWidth,m_tRenderInfo.nHeight,m_tRenderInfo.nYStride,m_tRenderInfo.nUVStride,m_tRenderInfo.nLineStride,m_tRenderInfo.sPixeFormat,m_tRenderInfo.nFlags);
		//截屏
		if (m_bScreenShot)
		{
			QString sFileName;
			QString sFileDir;
			quint64 uiTime;
			int nType;
			int nChl;
			m_bScreenShot=false;
			if (getScreenShotInfo(sFileName,sFileDir,uiTime,nChl,nType))
			{
				unsigned char *rgbBuff = new unsigned char[m_tRenderInfo.nWidth*m_tRenderInfo.nHeight*3];
				memset(rgbBuff, 0, m_tRenderInfo.nWidth*m_tRenderInfo.nHeight*3);
				YUV420ToRGB888((unsigned char*)m_tRenderInfo.pYdata, (unsigned char*)m_tRenderInfo.pUdata, (unsigned char*)m_tRenderInfo.pVdata,m_tRenderInfo.nWidth, m_tRenderInfo.nHeight, rgbBuff);
				QImage img(rgbBuff, m_tRenderInfo.nWidth, m_tRenderInfo.nHeight, QImage::Format_RGB888);
				QString sFilePath=sFileDir+"/"+sFileName;
				img.save(sFilePath, "JPG");
				delete [] rgbBuff;
				if (saveScreenShotInfoToDatabase(sFileName,sFileDir,uiTime,nChl,nType))
				{
					QVariantMap tScreenShotInfo;
					tScreenShotInfo.insert("fileName",sFileName);
					tScreenShotInfo.insert("fileDir",sFileDir);
					tScreenShotInfo.insert("chl",nChl);
					tScreenShotInfo.insert("type",nType);
					tScreenShotInfo.insert("user",m_sScreenUser);
					eventCallBack("screenShot",tScreenShotInfo);
					//do nothing
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"save screenShot info to database fail as saveScreenShotInfoToDatabase";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as get getScreenShotInfo fail";
				//do nothing
			}
		}
	}else{
		//do nothing
	}
	m_tVideoRenderLock.unlock();
}

int QSubviewRun::startManualRecord()
{
	m_bIsManualRecord=true;
	if (QThread::isRunning())
	{
		if (m_currentStatus==STATUS_CONNECTED)
		{
			m_tStepCodeLock.lock();
			m_stepCode.enqueue(SETMANUALRECORD);
			m_tStepCodeLock.unlock();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"startManualRecord fail as m_currentStatus!=STATUS_CONNECTED";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"startManualRecord fail as the thread is not running";
	}
	return 0;
}

int QSubviewRun::stopManualRecord()
{
	m_bIsManualRecord=false;
	if (QThread::isRunning())
	{
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(SETMANUALRECORD);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
	return 0;
}

int QSubviewRun::getRecordStatus()
{
	if (NULL!=m_pRecordDat)
	{
		return m_pRecordDat->getRecordStatus();
	}else{
		return 0;
	}
}

int QSubviewRun::cbCAuthority( QString evName,QVariantMap evMap,void*pUser )
{
	if (evMap.contains("Authority"))
	{
		eventCallBack("Authority",evMap);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"undefined callBack event,please checkout";
	}
	return 0;
}

int QSubviewRun::cbCMotionDetection( QString evName,QVariantMap evMap,void*pUser )
{
	if (evName=="MDSignal")
	{
		if (evMap.value("signal").toBool()==true)
		{
			m_tStepCodeLock.lock();
			if (!m_stepCode.contains(SETMOTIONRECORD))
			{
				m_stepCode.enqueue(SETMOTIONRECORD);
			}
			m_tStepCodeLock.unlock();
		}else{
			//do nothing
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"undefine evName:"<<evName;
	}
	return 0;
}

void QSubviewRun::enableStretch( bool bStretch )
{
	if (NULL != m_pIVideoRender)
	{
		m_pIVideoRender->enableStretch(bStretch);
	}
	m_bStretch = bStretch;
	if (QThread::isRunning())
	{
		//set nstepcode
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(VEDIOSTRETCH);
		m_tStepCodeLock.unlock();
	}
}

void QSubviewRun::enableStretch()
{
	IWindowSettings * pi;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IWindowSettings,(void **)&pi);
	if (NULL != pi)
	{
		pi->setEnableStretch(m_nWindId,m_bStretch);
		pi->Release();
	}
}

bool QSubviewRun::slAddExtendWnd()
{
	if (QThread::isRunning())
	{
		m_tDigitalZoomView.setGeometry(m_tDigitalZoomView.getPosition());
		m_tDigitalZoomView.show();
		m_tVideoRenderLock.lock();
		if (NULL!=m_pIVideoRender)
		{
			IVideoRenderDigitalZoom *pVideoRender=NULL;
			m_pIVideoRender->QueryInterface(IID_IVideoRenderDigitalZoom,(void**)&pVideoRender);
			if (NULL!=pVideoRender)
			{
				QString sName;
				pVideoRender->addExtendWnd(&m_tDigitalZoomView,sName);
				pVideoRender->Release();
				pVideoRender=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"addExtendWnd fail as pVideoRender is null";
			}
		}else{
			//do nothing
			qDebug()<<__FUNCTION__<<__LINE__<<"addExtendWnd fail as m_pIVideoRender is null";
		}
		m_tVideoRenderLock.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"addExtendWnd fail as the thread is not running";
		//do nothing
	}
	return true;
}


void QSubviewRun::slRemoveExtendWnd()
{
	m_tDigitalZoomView.close();
	m_tVideoRenderLock.lock();
	if (NULL!=m_pIVideoRender)
	{
		IVideoRenderDigitalZoom *pVideoRender=NULL;
		m_pIVideoRender->QueryInterface(IID_IVideoRenderDigitalZoom,(void**)&pVideoRender);
		if (NULL!=pVideoRender)
		{
			QString sName;
			pVideoRender->removeExtendWnd(sName);
			pVideoRender->Release();
			pVideoRender=NULL;
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
	m_tVideoRenderLock.unlock();
	emit sgShutDownDigtalZoom();
}
void QSubviewRun::drawRectToOriginalWnd( QPoint tStartPoint,QPoint tEndPoint )
{
	m_tVideoRenderLock.lock();
	if (NULL!=m_pIVideoRender)
	{
		IVideoRenderDigitalZoom *pVideoRender=NULL;
		m_pIVideoRender->QueryInterface(IID_IVideoRenderDigitalZoom,(void**)&pVideoRender);
		if (NULL!=pVideoRender)
		{
			pVideoRender->drawRectToOriginalWnd(tStartPoint.x(),tStartPoint.y(),tEndPoint.x(),tEndPoint.y());
			pVideoRender->Release();
			pVideoRender=NULL;
		}
	}else{
		//do nothing
	}
	m_tVideoRenderLock.unlock();
}
void QSubviewRun::SLSetRenderRect(QPoint tStartPoint,QPoint tEndPoint)
{
	m_tVideoRenderLock.lock();
	if (NULL!=m_pIVideoRender)
	{
		IVideoRenderDigitalZoom *pVideoRender=NULL;
		m_pIVideoRender->QueryInterface(IID_IVideoRenderDigitalZoom,(void**)&pVideoRender);
		if (NULL!=pVideoRender)
		{
			int nViewWidth;
			int nViewHeight;
			m_tDigitalZoomView.getDigitalViewSize(nViewWidth,nViewHeight);
			pVideoRender->setRenderRect(tStartPoint.x(),tStartPoint.y(),tEndPoint.x(),tEndPoint.y(),nViewWidth,nViewHeight);
			pVideoRender->Release();
			pVideoRender=NULL;
		}
	}else{
		//do nothing
	}
	m_tVideoRenderLock.unlock();
}

bool QSubviewRun::getDigtalViewIsClose()
{
	return m_tDigitalZoomView.getCurrentViewIsClose();
}

void QSubviewRun::deInitDigitalView()
{
	m_tVideoRenderLock.lock();
	if (NULL!=m_pIVideoRender)
	{
		IVideoRenderDigitalZoom *pVideoRender=NULL;
		m_pIVideoRender->QueryInterface(IID_IVideoRenderDigitalZoom,(void**)&pVideoRender);
		if (NULL!=pVideoRender)
		{
			QString sName;
			pVideoRender->removeExtendWnd(sName);
			pVideoRender->setRenderRect(0,0,0,0,0,0);
			m_tDigitalZoomView.clearRectPoint();
			pVideoRender->Release();
			pVideoRender=NULL;
		}
	}else{
		//do nothing
	}
	m_tVideoRenderLock.unlock();
}

bool QSubviewRun::isSuitForDigitalZoom()
{
	if (QThread::isRunning())
	{
		m_tVideoRenderLock.lock();
		if (NULL!=m_pIVideoRender)
		{
			IVideoRenderDigitalZoom *pVideoRender=NULL;
			m_pIVideoRender->QueryInterface(IID_IVideoRenderDigitalZoom,(void**)&pVideoRender);
			if (NULL!=pVideoRender)
			{
				pVideoRender->Release();
				pVideoRender=NULL;
				m_tVideoRenderLock.unlock();
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"addExtendWnd fail as pVideoRender is null";
			}
		}else{
			//do nothing
			qDebug()<<__FUNCTION__<<__LINE__<<"addExtendWnd fail as m_pIVideoRender is null";
		}
		m_tVideoRenderLock.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"addExtendWnd fail as the thread is not running";
		//do nothing
	}
	return false;
}

void QSubviewRun::showDigitalView()
{
	slAddExtendWnd();
}

void QSubviewRun::closeDigitalView()
{
	slRemoveExtendWnd();
}

void QSubviewRun::setDigitalZoomToMainStream()
{
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		//set nstepcode
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(DIGITALZOOMTOMAINSTREAM);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"DIGITALZOOMTOMAINSTREAM fail";
	}
}

void QSubviewRun::setDigitalZoomStreamRestore()
{
	if (QThread::isRunning()&&m_currentStatus==STATUS_CONNECTED)
	{
		//set nstepcode
		m_tStepCodeLock.lock();
		m_stepCode.enqueue(DIGITALZOOMSTREAMRESTORE);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
		//qDebug()<<__FUNCTION__<<__LINE__<<"DIGITALZOOMSTREAMRESTORE fail";
	}
}

void QSubviewRun::digitalZoomToMainStream()
{
	if (m_currentStatus==STATUS_CONNECTED&&NULL!=m_pdeviceClient)
	{
		if ("IPC"==m_tDeviceInfo.m_sVendor||m_tDeviceInfo.m_sVendor=="ONVIF")
		{
			//IPC 和ONVIF协议的码流切换
			ISwitchStream *pSwitchStream=NULL;
			m_pdeviceClient->QueryInterface(IID_ISwitchStream,(void**)&pSwitchStream);
			if (NULL!=pSwitchStream)
			{
				pSwitchStream->SwitchStream(0);
				pSwitchStream->Release();
				pSwitchStream=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail as apply ISwitchStream fail";
			}
		}else{
			IDeviceClient *pdeviceClient=NULL;
			m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdeviceClient);
			if (NULL !=pdeviceClient)
			{
				m_nSecondPosition=__LINE__;
				if (pdeviceClient->liveStreamRequire(m_tDeviceInfo.m_uiChannelId,0,true)==0)
				{
					pdeviceClient->Release();
					pdeviceClient=NULL;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail: "<<1;
					pdeviceClient->Release();
					pdeviceClient=NULL;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail as device client do not support IDeviceClient interface";
			}
		}
		if (m_tDeviceInfo.m_uiStreamId!=0)
		{
			m_tDigitalHisStreamNum=m_tDeviceInfo.m_uiStreamId;
			IChannelManager *pChannelManger=NULL;
			pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IChannelManager,(void**)&pChannelManger);
			if (NULL!=pChannelManger)
			{
				pChannelManger->ModifyChannelStream(m_tDeviceInfo.m_uiChannelIdInDataBase,0);
				m_tDeviceInfo.m_uiStreamId=0;
				pChannelManger->Release();
				pChannelManger=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"saveToDataBase fail as pChannelManger is null ";
			}
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail";
	}
}

void QSubviewRun::digitalZoomStreamRestore()
{
	if (m_currentStatus==STATUS_CONNECTED&&NULL!=m_pdeviceClient)
	{
		if (m_tDigitalHisStreamNum!=0)
		{
			IChannelManager *pChannelManger=NULL;
			pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IChannelManager,(void**)&pChannelManger);
			if (NULL!=pChannelManger)
			{
				pChannelManger->ModifyChannelStream(m_tDeviceInfo.m_uiChannelIdInDataBase,m_tDigitalHisStreamNum);
				m_tDeviceInfo.m_uiStreamId=m_tDigitalHisStreamNum;
				pChannelManger->Release();
				pChannelManger=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"saveToDataBase fail as pChannelManger is null ";
			}
		}
		if ("IPC"==m_tDeviceInfo.m_sVendor||m_tDeviceInfo.m_sVendor=="ONVIF")
		{
			//IPC 和ONVIF协议的码流切换
			ISwitchStream *pSwitchStream=NULL;
			m_pdeviceClient->QueryInterface(IID_ISwitchStream,(void**)&pSwitchStream);
			if (NULL!=pSwitchStream)
			{
				pSwitchStream->SwitchStream(m_tDeviceInfo.m_uiStreamId);
				pSwitchStream->Release();
				pSwitchStream=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail as apply ISwitchStream fail";
			}
		}else{
			IDeviceClient *pdeviceClient=NULL;
			m_pdeviceClient->QueryInterface(IID_IDeviceClient,(void**)&pdeviceClient);
			if (NULL !=pdeviceClient)
			{
				m_nSecondPosition=__LINE__;
				if (pdeviceClient->liveStreamRequire(m_tDeviceInfo.m_uiChannelId,m_tDeviceInfo.m_uiStreamId,true)==0)
				{
					pdeviceClient->Release();
					pdeviceClient=NULL;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail: "<<1;
					pdeviceClient->Release();
					pdeviceClient=NULL;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"liveSteamRequire fail as device client do not support IDeviceClient interface";
			}
		}

	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"SWITCHSTREAMEX fail";
	}
}

void QSubviewRun::initDigitalRect( QPoint tStartPoint,QPoint tEndPoint,int nWidth,int nHeight )
{
	int nViewWidth;
	int nViewHeight;
	m_tDigitalZoomView.getDigitalViewSize(nViewWidth,nViewHeight);
	QPoint tToViewStartPoint;
	QPoint tToViewEndPoint;
	tToViewStartPoint.setX(tStartPoint.x()*nViewWidth/nWidth);
	tToViewStartPoint.setY(tStartPoint.y()*nViewHeight/nHeight);
	tToViewEndPoint.setX(tEndPoint.x()*nViewWidth/nWidth);
	tToViewEndPoint.setY(tEndPoint.y()*nViewHeight/nHeight);
	SLSetRenderRect(tToViewStartPoint,tToViewEndPoint);
	m_tDigitalZoomView.initRectPoint(tToViewStartPoint,tToViewEndPoint);
}

void QSubviewRun::setParentWnd( QWidget*wnd )
{
	m_tDigitalZoomView.setParentWnd(wnd);
}

void QSubviewRun::ViewNewPosition( QRect tRect,int nWidth,int nHeight )
{
	m_tDigitalZoomView.ViewNewPosition(tRect,nWidth,nHeight);
}

bool QSubviewRun::getScreenShotInfo( QString &sFileName,QString &sFileDir,quint64 &uiTime,int &nChl,int &nType )
{
	IDisksSetting *pDisksSetting=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		QString sDisk;
		if (0==pDisksSetting->getUseDisks(sDisk))
		{
			QStringList tDiskList=sDisk.split(":");
			if (tDiskList.size()!=0)
			{
				foreach(QString sDiskItem,tDiskList){
					QString sDiskEx=sDiskItem+":/screenShotEx";
					QDir tDir;
					if (tDir.exists(sDiskEx))
					{
						sFileDir=sDiskEx;
						break;
					}else{
						//create dir
						if (tDir.mkdir(sDiskEx))
						{
							sFileDir=sDiskEx;
							break;
						}else{
							//keep going
						}
					}
				}
				if (!sFileDir.isEmpty())
				{
					nType=m_nScreenShotType;
					nChl=m_nScreenShotChl;
					uiTime=QDateTime::currentDateTime().toMSecsSinceEpoch();
					QString sDatetime=QDateTime::currentDateTime().toString("yyyy-MM-dd")+"-"+QDateTime::currentDateTime().toString("hh-mm-ss-zzz");
					sFileName=m_sScreenUser+"-"+QString::number(nChl)+"-"+QString::number(nType)+'-'+sDatetime+".jpg";
					pDisksSetting->Release();
					pDisksSetting=NULL;
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as sFileDir is not been created";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as there is not disk for store screen";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as getUseDisks fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as pDisksSetting is null";
	}
	if (NULL!=pDisksSetting)
	{
		pDisksSetting->Release();
		pDisksSetting=NULL;
	}
	return false;
}

bool QSubviewRun::saveScreenShotInfoToDatabase( QString sFileName,QString sFileDir ,quint64 uiTime,int nChl,int nType )
{
	IScreenShot *pScreenShot=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IScreenShot,(void**)&pScreenShot);
	if (NULL!=pScreenShot)
	{
		if (pScreenShot->addScreenShotItem(sFileName,sFileDir,m_sScreenUser,nChl,nType,uiTime))
		{
			pScreenShot->Release();
			pScreenShot=NULL;
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"saveScreenShotInfoToDatabase fail as addScreenShotItem fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"saveScreenShotInfoToDatabase fail as pScreenShot is null";
	}
	if (NULL!=pScreenShot)
	{
		pScreenShot->Release();
		pScreenShot=NULL;
	}
	return false;
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

int cbConnectRefuse( QString evName,QVariantMap evMap,void*pUser )
{
	return ((QSubviewRun*)pUser)->cbCConnectRefuse(evName,evMap,pUser);
}

int cbAuthority( QString evName,QVariantMap evMap,void*pUser )
{
	return ((QSubviewRun*)pUser)->cbCAuthority(evName,evMap,pUser);
}

int cbMotionDetection( QString evName,QVariantMap evMap,void*pUser )
{
	return ((QSubviewRun*)pUser)->cbCMotionDetection(evName,evMap,pUser);
}
