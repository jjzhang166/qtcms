#include "qsubview.h"
#include <guid.h>
#include <QtGui/QPainter>
#include <QPoint>
#include <QtCore>
#include <QSettings>
#include <QMouseEvent>
#include <QtXml/QtXml>
#include "ILocalSetting.h"
#include <IChannelManager.h>
#include <IDeviceManager.h>
#include <QDir>
#include <QDateTime>
#include <QApplication>
bool QSubView::m_bIsAudioOpend = false;
IAudioPlayer* QSubView::m_pAudioPlayer = NULL;
QSubView* QSubView::m_pCurrView = NULL;
int QSubView::m_nSampleRate=0;
int QSubView::m_nSampleWidth=0;

QSubView::QSubView(QWidget *parent)
	: QWidget(parent),
	m_IVideoRender(NULL),
	m_IVideoDecoder(NULL),
	m_IDeviceClientDecideByVendor(NULL),
	m_pRecorder(NULL),
	m_pRecordTime(NULL),
	m_pPTZControl(NULL),
	m_CurrentState(QSubView::STATUS_DISCONNECTED),
	m_HistoryState(QSubView::STATUS_DISCONNECTED),
	iInitWidth(0),
	iInitHeight(0),
	m_bRendering(false),
	m_bIsRecording(false),
	m_bStateAutoConnect(false),
	m_bIsAutoConnecting(false),
	m_bIsStartRecording(false),
	m_bIsAutoRecording(false),
	m_bIsFocus(false),
	m_bIsForbidConnect(false),
	m_bIsPTZAutoOpened(false),
	ui(new Ui::titleview),
	m_QActionCloseView(NULL),
	m_QActionSwitchStream(NULL),
	_translator(NULL),
	m_nCountDisConnecting(0),
	m_CountConnecting(0),
	m_DisConnectingTimeId(0),
	m_RenderTimeId(0),
	m_AutoConnectTimeId(0),
	m_RecordFlushTime(0)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);
	//申请解码器接口
	pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_IVideoDecoder);
// 	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_IVideoDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_IVideoRender);
	//申请IRecorder接口
	pcomCreateInstance(CLSID_Recorder,NULL,IID_IRecorder,(void **)&m_pRecorder);

	//申请ISetRecordTime接口
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ISetRecordTime,(void **)&m_pRecordTime);
	connect(&m_checkTime, SIGNAL(timeout()), this, SLOT(OnCheckTime()));

	connect(this,SIGNAL(DisConnecting()),this,SLOT(OnDisConnecting()),Qt::QueuedConnection);
	connect(this,SIGNAL(Connectting()),this,SLOT(OnConnectting()),Qt::QueuedConnection);//Ui显示正在连接中
	connect(this,SIGNAL(RenderHistoryPix()),this,SLOT(OnRenderHistoryPix()),Qt::QueuedConnection);
	connect(this,SIGNAL(AutoConnectSignals()),this,SLOT(In_OpenAutoConnect()),Qt::QueuedConnection);
	connect(this,SIGNAL(CreateAutoConnectTimeSignals()),this,SLOT(OnCreateAutoConnectTime()),Qt::QueuedConnection);
	connect(this,SIGNAL(Connectting()),this,SLOT(OnConnected()),Qt::QueuedConnection);
	m_QActionCloseView=m_RMousePressMenu.addAction(tr("Close Preview"));
	m_QActionSwitchStream=m_RMousePressMenu.addAction(tr("Switch Stream"));
	connect(this,SIGNAL(RMousePressMenu()),this,SLOT(OnRMousePressMenu()));
	connect(m_QActionCloseView,SIGNAL(triggered(bool)),this,SLOT(OnCloseFromMouseEv()));
	connect(m_QActionSwitchStream,SIGNAL(triggered(bool)),this,SLOT(OnSwitchStreamFromMouseEv()));

	//初始历史render的值
	m_ForbidConnectTimeId=startTimer(2000);
	m_HistoryRenderInfo.pData=NULL;
	//test
	_manageWidget=new ManageWidget(this);
	connect(this,SIGNAL(RecordStateSignals(bool)),_manageWidget,SLOT(RecordState(bool)));
	initDeviceInfo();
	_translator=new QTranslator();
	QApplication::installTranslator(_translator);
}

QSubView::~QSubView()
{
	CloseWndCamera();
	
	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		m_IDeviceClientDecideByVendor->Release();
		m_IDeviceClientDecideByVendor=NULL;
	}

	if (NULL!=m_IVideoDecoder)
	{
		m_IVideoDecoder->Release();
	}
	if (NULL!=m_IVideoRender)
	{
		m_IVideoRender->Release();
	}
	delete ui;

	m_checkTime.stop();
	if (NULL != m_pRecordTime)
	{
		m_pRecordTime->Release();
	}

	if (NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_pRecorder->Release();
	}
	if (m_ForbidConnectTimeId!=0)
	{
		killTimer(m_ForbidConnectTimeId);
	}
	if (NULL != m_pAudioPlayer)
	{
		m_pAudioPlayer->Stop();
		m_pAudioPlayer->Release();
		m_pAudioPlayer = NULL;
	}
	if (NULL!=_translator)
	{
		delete _translator;
		_translator=NULL;
	}	
}


void QSubView::paintEvent( QPaintEvent * e)
{
	paintEventCache(e);
	paintEventConnecting(e);
	paintEventNoVideo(e);
}

void QSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}



void QSubView::mousePressEvent(QMouseEvent *ev)
{
	setFocus(Qt::MouseFocusReason);
	saveCacheImage();
	if (ev->button()==Qt::RightButton)
	{
		emit RMousePressMenu();
	}
	emit mousePressEvent(this,ev);
	emit SetCurrentWindSignl(this);
	if (m_bIsAudioOpend && ev->button() == Qt::LeftButton)
	{
		m_pCurrView = this;
	}
}

int QSubView::GetCurrentWnd()
{
	return 1;
}

int QSubView::OpenCameraInWnd(int chlId)
{
	//禁止频繁操作
	
	if (m_bIsForbidConnect==true)
	{
		return 1;
	}
	m_bIsForbidConnect=true;

	//设置自动手动关闭标志位
	if (m_bStateAutoConnect==true)
	{
		In_CloseAutoConnect();
	}
	//关闭上一次的连接
	//CloseWndCamera();
	//生成设备组件
	IDeviceClient *iDeviceClient=m_QSubViewObject.SetDeviceByVendor(m_DevCliSetInfo.m_sVendor,this);
	if (iDeviceClient!=NULL&&m_IDeviceClientDecideByVendor==NULL)
	{
		iDeviceClient->QueryInterface(IID_IDeviceClient,(void**)&m_IDeviceClientDecideByVendor);
	}
	//注册事件，需检测是否注册成功
		if (1==cbInit())
		{
			if (NULL!=m_IDeviceClientDecideByVendor)
			{
				m_IDeviceClientDecideByVendor->Release();
				m_IDeviceClientDecideByVendor=NULL;
			}
			return 1;
		}
	m_QSubViewObject.SetCameraInWnd(m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sEseeId,m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,m_DevCliSetInfo.m_sUsername,m_DevCliSetInfo.m_sPassword,m_DevCliSetInfo.m_sCameraname,m_DevCliSetInfo.m_sVendor);
	//IPC Time Synchronization
	if ("JUAN IPC" == m_DevCliSetInfo.m_sVendor)
	{
		ILocalSetting *pLocalSetting = NULL;
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void **)&pLocalSetting);
		if (NULL != pLocalSetting)
		{
			bool syncTime = pLocalSetting->getAutoSyncTime();
			pLocalSetting->Release();
			m_QSubViewObject.SetAutoSyncTime(syncTime);
		}
	}

	//0.5s检测一次是否需要刷新历史图片
	m_RenderTimeId=startTimer(500);
	m_QSubViewObject.OpenCameraInWnd();
	m_checkTime.start(1000);
	return 0;
}
int QSubView::CloseWndCamera()
{
	In_CloseAutoConnect();
	m_QSubViewObject.CloseWndCamera();
	//释放动态生成的指针
	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		m_IDeviceClientDecideByVendor->Release();
		m_IDeviceClientDecideByVendor=NULL;
	}
	//手动录像
	if (m_bIsRecording && NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_bIsRecording = false;
	}
	//计划录像
	if (m_bIsAutoRecording && NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_bIsAutoRecording = false;
	}
	m_checkTime.stop();
	m_RecordFlushTime=0;

	return 0;
}
int QSubView::GetWindowConnectionStatus()
{
	return m_CurrentState;
}


int QSubView::cbInit()
{
	//注册设备服务回调函数
	QString evName="LiveStream";
	IEventRegister *pRegist=NULL;
	if (NULL==m_IDeviceClientDecideByVendor)
	{
		return 1;
	}
	m_IDeviceClientDecideByVendor->QueryInterface(IID_IEventRegister,(void**)&pRegist);
	if (NULL==pRegist)
	{
		return 1;
	}
	pRegist->registerEvent(evName,cbLiveStream,this);
	evName.clear();
	evName.append("SocketError");
	pRegist->registerEvent(evName,cbConnectError,this);
	evName.clear();
	evName.append("CurrentStatus");
	pRegist->registerEvent(evName,cbStateChange,this);

	evName.clear();
	evName.append("ForRecord");
	pRegist->registerEvent(evName,cbForRecord,this);

	pRegist->Release();
	pRegist=NULL;
	//注册解码回调函数
	evName.clear();
	evName.append("DecodedFrame");
	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}
	m_IVideoDecoder->QueryInterface(IID_IEventRegister,(void**)&pRegist);
	if (NULL==pRegist)
	{
		return 1;
	}
	pRegist->registerEvent(evName,cbDecodedFrame,this);
	pRegist->Release();
	pRegist=NULL;
	//初始化渲染器
	if (NULL==m_IVideoRender)
	{
		return 1;
	}
	if (0!=m_IVideoRender->setRenderWnd(_manageWidget->GetWidgetForVideo()))
	{
		return 1;
	}
	//设置录像状态回到函数
	if (NULL!=m_pRecorder)
	{
		m_pRecorder->QueryInterface(IID_IEventRegister,(void**)&pRegist);
		if (NULL!=pRegist)
		{
			pRegist->registerEvent("RecordState",cbRecordState,this);
			pRegist->Release();
			pRegist=NULL;
		}
	}
	return 0;
}
int QSubView::PrevPlay(QVariantMap evMap)
{
	unsigned int nLength=evMap.value("length").toUInt();
	char * lpdata=(char *)evMap.value("data").toUInt();
	int frameType = evMap.value("frametype").toUInt();

	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}

	if (NULL != m_pAudioPlayer && 0 == frameType && m_pCurrView == this)
	{
		int nSampleRate = evMap.value("samplerate").toUInt();
		int nSampleWidth = evMap.value("samplewidth").toUInt();
		if (nSampleRate != m_nSampleRate || nSampleWidth != m_nSampleWidth)
		{
			m_nSampleRate = nSampleRate;
			m_nSampleWidth = nSampleWidth;
			m_pAudioPlayer->SetAudioParam(1, m_nSampleRate, m_nSampleWidth);
		}
		m_pAudioPlayer->Play(lpdata, nLength);
	}

	m_IVideoDecoder->decode(lpdata,nLength);
	return 0;
}
int QSubView::CurrentStateChange(QVariantMap evMap)
{

	m_CurrentState=(QSubViewConnectStatus)evMap.value("CurrentStatus").toInt();
	
	
	if (1==m_CurrentState)
	{
		emit Connectting();
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"connecting";
	}
	if (0==m_CurrentState)
	{
		emit Connected();
		m_bIsAutoConnect=true;
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		

		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"connected";
	}
	if (3==m_CurrentState)
	{
		emit DisConnecting();
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"disconnecting";
	}
	if (2==m_CurrentState)
	{
		//断开后，把历史图片数据清空
		m_HistoryRenderInfo.pData=NULL;
		update();
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);

		//手动录像
		if (m_bIsRecording && NULL != m_pRecorder)
		{
			m_pRecorder->Stop();
			m_bIsRecording = false;
		}
		//计划录像
		if (m_bIsAutoRecording && NULL != m_pRecorder)
		{
			m_pRecorder->Stop();
			m_bIsAutoRecording = false;
		}
		m_checkTime.stop();
		m_RecordFlushTime=0;
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"disconnected";
	}
	
	
	/*emit CurrentStateChangeSignl(evMap.value("CurrentStatus").toInt(),this);*/
	//自动重连
    if (STATUS_DISCONNECTED==m_CurrentState&&STATUS_CONNECTED==m_HistoryState)
	{
		if (m_bIsAutoConnect==true)
		{
			if (m_bStateAutoConnect==false)
			{
				//开启自动重连时钟
				emit CreateAutoConnectTimeSignals();
				m_bStateAutoConnect=true;
			}
			else{

			}
		}
	}

	m_HistoryState=m_CurrentState;
	return 0;
}
int QSubView::PrevRender(QVariantMap evMap)
{
	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
	}
	if (NULL==m_IVideoRender)
	{
		return 1;
	}

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

	if (iInitHeight!=iHeight||iInitWidth!=iWidth)
	{
		m_IVideoRender->init(iWidth,iHeight);
		iInitHeight=iHeight;
		iInitWidth=iWidth;
	}
	m_bRendering=true;
	m_csRender.lock();
	m_HistoryRenderInfo.pData=pData;
	m_HistoryRenderInfo.pYdata=pYdata;
	m_HistoryRenderInfo.pUdata=pUdata;
	m_HistoryRenderInfo.pVdata=pVdata;
	m_HistoryRenderInfo.iWidth=iWidth;
	m_HistoryRenderInfo.iHeight=iHeight;
	m_HistoryRenderInfo.iYStride=iYStride;
	m_HistoryRenderInfo.iUVStride=iUVStride;
	m_HistoryRenderInfo.iLineStride=iLineStride;
	m_HistoryRenderInfo.iPixeFormat=iPixeFormat;
	m_HistoryRenderInfo.iFlags=iFlags;
	m_bIsRenderHistory=false;
	m_IVideoRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);
	m_csRender.unlock();
	m_bRendering=false;
	return 0;
}



void QSubView::timerEvent( QTimerEvent * ev)
{

	//频繁操作
	if (m_ForbidConnectTimeId==ev->timerId())
	{
		m_bIsForbidConnect=false;
	}
	//连接ing
	if (m_CurrentState==STATUS_CONNECTING&&ev->timerId()==m_ConnectingTimeId)
	{
		update();
	}else if (m_CurrentState!=STATUS_CONNECTING&&ev->timerId()==m_ConnectingTimeId)
	{
		killTimer(ev->timerId());
		m_ConnectingTimeId=0;
	}

	if (m_CurrentState!=STATUS_DISCONNECTING&&ev->timerId()==m_DisConnectingTimeId)
	{
		killTimer(ev->timerId());
		m_DisConnectingTimeId=0;
	}
	if (m_CurrentState!=STATUS_CONNECTED&&ev->timerId()==m_RenderTimeId)
	{
		killTimer(ev->timerId());
		m_RenderTimeId=0;
	}
	if (m_RenderTimeId==ev->timerId())
	{
		emit RenderHistoryPix();
	}
	//自动重连 时钟信号
	if (m_AutoConnectTimeId==ev->timerId())
	{
		//关闭自动重连,设备连接上或者禁止自动重连
        if (m_bIsAutoConnect==false||STATUS_CONNECTED==m_CurrentState)
		{
			killTimer(ev->timerId());
			m_AutoConnectTimeId=0;
			m_bStateAutoConnect=false;
			//判定是否需要重新录像
            if (STATUS_CONNECTED==m_CurrentState&&true==m_bIsStartRecording)
			{
				if (NULL != m_pRecorder&&m_bIsAutoRecording==false)
				{
					m_bIsRecording = true;
					m_pRecorder->SetDevInfo(m_RecordDevInfo.m_DevName,m_RecordDevInfo.m_ChannelNum);
					m_pRecorder->Start();
				}
			}
		}
		else if (m_bIsAutoConnect==true)
		{
            if (STATUS_CONNECTED!=m_CurrentState&&false==m_bIsAutoConnecting)
			{
				qDebug()<<"AutoConnectSignals";
				killTimer(ev->timerId());
				m_AutoConnectTimeId=startTimer(10000);
				emit AutoConnectSignals();
			}
		}
	}
}

void QSubView::OnRMousePressMenu()
{
	if (m_DevCliSetInfo.m_uiStreamId==0)
	{
		m_QActionSwitchStream->setText(tr("Switch to SubStream"));
	}else{
		m_QActionSwitchStream->setText(tr("Switch to MainStream"));
	}

	if (m_CurrentState==STATUS_DISCONNECTED)
	{
		m_QActionCloseView->setDisabled(true);
		m_QActionSwitchStream->setDisabled(true);
	}else{
		m_QActionCloseView->setEnabled(true);
		m_QActionSwitchStream->setEnabled(true);
	}
	m_RMousePressMenu.exec(QCursor::pos());
}

void QSubView::OnCloseFromMouseEv()
{
	CloseWndCamera();
}
void QSubView::OnSwitchStreamFromMouseEv()
{
	if (m_CurrentState==STATUS_CONNECTED)
	{
		if (m_DevCliSetInfo.m_uiStreamId==0)
		{
			liveStreamRequire(m_DevCliSetInfo.m_uiChannelId,1,true);
			m_DevCliSetInfo.m_uiStreamId=1;
			SaveToDatobase();
		}else{
			liveStreamRequire(m_DevCliSetInfo.m_uiChannelId,0,true);
			m_DevCliSetInfo.m_uiStreamId=0;
			SaveToDatobase();
	}
	}
}

QSubView* QSubView::getCurWind()
{
	return m_pCurrView;
}
int QSubView::AudioEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		//申请IAudioPlayer接口
		pcomCreateInstance(CLSID_AudioPlayer,NULL,IID_IAudioPlayer,(void **)&m_pAudioPlayer);
		if (NULL != m_pAudioPlayer)
		{
			m_pAudioPlayer->EnablePlay(true);
		}
		m_bIsAudioOpend = true;
	}
	else
	{
		if (NULL != m_pAudioPlayer)
		{
			m_pAudioPlayer->Stop();
			m_pAudioPlayer->Release();
			m_pAudioPlayer = NULL;
			m_bIsAudioOpend = false;
			m_pCurrView = NULL;
			m_nSampleRate = 0;
			m_nSampleWidth = 0;
		}
	}
	return 0;
}
int cbLiveStream(QString evName,QVariantMap evMap,void*pUser)
{
	//检测数据包，把数据包扔给解码器

	if (evName=="LiveStream")
	{
		((QSubView*)pUser)->PrevPlay(evMap);
		return 0;
	}
	else
		return 1;
}
int cbForRecord(QString evName,QVariantMap evMap,void*pUser)
{
	//检测数据包，把数据包扔给解码器

	if (evName=="ForRecord")
	{
		((QSubView*)pUser)->ForRecord(evMap);
		return 0;
	}
	else
		return 1;
}
int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser)
{
	if (evName=="DecodedFrame")
	{
		((QSubView*)pUser)->PrevRender(evMap);
		return 0;
	}
	else 
		return 1;
}

int cbConnectError(QString evName,QVariantMap evMap,void*pUser)
{
    Q_UNUSED(evName);
    Q_UNUSED(pUser);
	qDebug()<<"cbConnectError";

	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
	}
	return 1;
}

int cbStateChange(QString evName,QVariantMap evMap,void*pUser)
{
	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
	}
	if (evName=="CurrentStatus")
	{
		((QSubView*)pUser)->CurrentStateChange(evMap);
		return 0;
	}
	return 1;
}

int cbRecordState( QString evName,QVariantMap evMap,void*pUser )
{
	if (evName=="RecordState")
	{
		((QSubView*)pUser)->RecordState(evMap);
		return 0;
	}
	else
		return 1;
}

int QSubView::StartRecord()
{
	if (NULL == m_pRecorder || m_bIsAutoRecording)
	{
		if (m_bIsAutoRecording)
		{
			return 2;
		}else{
			return 1;
		}
	}
	m_bIsStartRecording=true;
	m_bIsRecording = true;
	int nRet = m_pRecorder->Start();
	return nRet;
}

int QSubView::StopRecord()
{
	if (NULL == m_pRecorder || m_bIsAutoRecording)
	{
		if (m_bIsAutoRecording)
		{
			return 2;
		}else{
			return 1;
		}
	}
	int nRet = m_pRecorder->Stop();
	m_bIsRecording = false;
	m_bIsStartRecording=false;
	return nRet;
}

int QSubView::SetDevInfo(const QString &devname,int nChannelNum)
{
	if (NULL == m_pRecorder)
	{
		return 1;
	}
	m_RecordDevInfo.m_DevName.clear();
	m_RecordDevInfo.m_ChannelNum=nChannelNum;
	m_RecordDevInfo.m_DevName=devname;
	int nRet = m_pRecorder->SetDevInfo(devname, nChannelNum);
	return nRet;
}
int QSubView::SetPlayWnd(int nWnd)
{
	if (NULL == m_pAudioPlayer)
	{
		return 1;
	}
	m_pAudioPlayer->SetPlayWnd(nWnd);
	return 0;
}	
int QSubView::SetVolume(unsigned int uiPersent)
{
	if (NULL == m_pAudioPlayer)
	{
		return 1;
	}
	m_pAudioPlayer->SetVolume(uiPersent);
	return 0;
}

void QSubView::OnCheckTime()
{
	if (NULL == m_pRecorder || m_bIsRecording)
	{
		return;
	}

	if (NULL == m_pRecordTime)
	{
		//重新申请ISetRecordTime接口
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ISetRecordTime,(void **)&m_pRecordTime);
		if (NULL == m_pRecordTime)
		{
			return;
		}else{
			IEventRegister *registe=NULL;
			m_pRecordTime->QueryInterface(IID_IEventRegister,(void**)&registe);
			if (NULL!=registe)
			{
				registe->registerEvent("RecordState",cbRecordState,this);
				registe->Release();
			}
		}
	}

	QTime currentTime; 
	RecordTimeInfo recTimeInfo;
	if (0 == m_RecordFlushTime%600)
	{
		m_RecordFlushTime = 0;
		++m_RecordFlushTime;
		m_lstReocrdTimeInfoList.clear();
		QStringList recordIdList = m_pRecordTime->GetRecordTimeBydevId(m_DevCliSetInfo.m_uiChannelIdInDataBase);
		for (int i = 0; i < recordIdList.size(); i++)
		{
			QString recordID = recordIdList[i];
			QVariantMap timeInfo = m_pRecordTime->GetRecordTimeInfo(recordID.toInt());
			recTimeInfo.nEnable = timeInfo.value("enable").toInt();
			recTimeInfo.nWeekDay = timeInfo.value("weekday").toInt();
			int weekDay = QDate::currentDate().dayOfWeek() - 1;
			if (0 == recTimeInfo.nEnable || weekDay != recTimeInfo.nWeekDay)
			{
				continue;
			}

			currentTime = QTime::currentTime();
			recTimeInfo.startTime = QTime::fromString(timeInfo.value("starttime").toString().mid(11), "hh:mm:ss");
			recTimeInfo.endTime = QTime::fromString(timeInfo.value("endtime").toString().mid(11), "hh:mm:ss");
			if (!m_bIsAutoRecording && currentTime >= recTimeInfo.startTime && currentTime < recTimeInfo.endTime)
			{
				//if (m_DevCliSetInfo.m_sEseeId.toInt()<1)
				//{
				//	m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sAddress, m_DevCliSetInfo.m_uiChannelId);
				//}else{
				//	m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sEseeId, m_DevCliSetInfo.m_uiChannelId);
				//}
				m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sDeviceName,m_DevCliSetInfo.m_uiChannelId);
				m_pRecorder->Start();
				m_bIsAutoRecording = true;
			}

			if (m_bIsAutoRecording && currentTime >= recTimeInfo.endTime)
			{
				m_pRecorder->Stop();
				m_bIsAutoRecording = false;
			}
			m_lstReocrdTimeInfoList.append(recTimeInfo);
		}
	}
	else
	{
		++m_RecordFlushTime;
		for (int j = 0; j < m_lstReocrdTimeInfoList.size(); ++j)
		{
			if (0 == m_lstReocrdTimeInfoList[j].nEnable || QDate::currentDate().dayOfWeek() - 1 != m_lstReocrdTimeInfoList[j].nWeekDay)
			{
				continue;
			}
			currentTime = QTime::currentTime();
			if (!m_bIsAutoRecording && currentTime >= m_lstReocrdTimeInfoList[j].startTime && currentTime < m_lstReocrdTimeInfoList[j].endTime&&m_CurrentState==STATUS_CONNECTED)
			{
				/*m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sEseeId, m_DevCliSetInfo.m_uiChannelId);*/
				m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sDeviceName,m_DevCliSetInfo.m_uiChannelId);
				m_pRecorder->Start();
				m_bIsAutoRecording = true;
			}
			if (m_bIsAutoRecording && currentTime >= m_lstReocrdTimeInfoList[j].endTime)
			{
				m_pRecorder->Stop();
				m_bIsAutoRecording = false;
			}
		}
		if (!m_lstReocrdTimeInfoList.size())
		{
			if (m_bIsAutoRecording)
			{
				m_pRecorder->Stop();
				m_bIsAutoRecording = false;
			}
		}
	}
}

int QSubView::ForRecord( QVariantMap evMap )
{
	if (NULL != m_pRecorder)
	{
		m_pRecorder->InputFrame(evMap);
	}
	return 0;
}



void QSubView::In_OpenAutoConnect()
{
	//关闭上一次的连接
	m_bIsAutoConnecting=true;
	m_QSubViewObject.CloseWndCamera();
	//释放动态生成的指针
	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		m_IDeviceClientDecideByVendor->Release();
		m_IDeviceClientDecideByVendor=NULL;
	}
	//录像
	if (m_bIsRecording && NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_bIsRecording = false;
	}
	//生成设备组件
	//SetDeviceByVendor(m_DevCliSetInfo.m_sVendor);

	IDeviceClient *iDeviceClient=m_QSubViewObject.SetDeviceByVendor(m_DevCliSetInfo.m_sVendor,this);
	if (iDeviceClient!=NULL&&m_IDeviceClientDecideByVendor==NULL)
	{
		iDeviceClient->QueryInterface(IID_IDeviceClient,(void**)&m_IDeviceClientDecideByVendor);
	}
	//注册事件，需检测是否注册成功
	if (1==cbInit())
	{
		if (NULL!=m_IDeviceClientDecideByVendor)
		{
			m_IDeviceClientDecideByVendor->Release();
			m_IDeviceClientDecideByVendor=NULL;
		}
		m_bIsAutoConnecting=false;
		return ;
	}
	m_QSubViewObject.SetCameraInWnd(m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sEseeId,m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,m_DevCliSetInfo.m_sUsername,m_DevCliSetInfo.m_sPassword,m_DevCliSetInfo.m_sCameraname,m_DevCliSetInfo.m_sVendor);

	m_QSubViewObject.OpenCameraInWnd();

	m_checkTime.start(1000);
	m_bIsAutoConnecting=false;
}

void QSubView::OnConnectting()
{
	m_ConnectingTimeId=startTimer(500);
}

void QSubView::OnDisConnecting()
{
    m_DisConnectingTimeId=startTimer(500);
}
void QSubView::OnConnected()
{
	m_RenderTimeId=startTimer(500);
}


void QSubView::OnRenderHistoryPix()
{
    if (m_CurrentState==STATUS_CONNECTED)
	{
		if (NULL!=m_IVideoRender&&NULL!=m_HistoryRenderInfo.pData&&true==m_bIsRenderHistory)
		{
			m_csRender.lock();
			m_IVideoRender->render(m_HistoryRenderInfo.pData,m_HistoryRenderInfo.pYdata,m_HistoryRenderInfo.pUdata,m_HistoryRenderInfo.pVdata,m_HistoryRenderInfo.iWidth,m_HistoryRenderInfo.iHeight,m_HistoryRenderInfo.iYStride,m_HistoryRenderInfo.iUVStride,m_HistoryRenderInfo.iLineStride,m_HistoryRenderInfo.iPixeFormat,m_HistoryRenderInfo.iFlags);
			m_csRender.unlock();
		}
		/*m_bIsRenderHistory=true;*/
	}
}

void QSubView::In_CloseAutoConnect()
{
	//关闭自动重连的时钟
	if (m_AutoConnectTimeId!=0)
	{
		killTimer(m_AutoConnectTimeId);
		m_AutoConnectTimeId=0;
	}
	//关闭连接
	m_bStateAutoConnect=false;
	m_bIsAutoConnect=false;
}

void QSubView::OnCreateAutoConnectTime()
{
	m_AutoConnectTimeId=startTimer(1000);
}

int QSubView::SetDevChannelInfo( int ChannelId )
{
	m_DevCliSetInfo.m_uiChannelIdInDataBase=ChannelId;
	GetDeviceInfo(ChannelId);
	return 0;
}

void QSubView::SetCurrentFocus( bool focus)
{
	if (m_bIsFocus==true)
	{
		update();
		
	}
	m_bIsFocus=focus;
	if (focus==true&&m_bIsAudioOpend)
	{
		m_pCurrView = this;
	}
}

void QSubView::resizeEvent( QResizeEvent * )
{
	_manageWidget->resize(this->size());
	if (this->parentWidget()->width()-this->width()<20)
	{
		if (NULL!=m_IDeviceClientDecideByVendor)
		{
			ISwitchStream *m_SwitchStream=NULL;
			m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
			if (NULL!=m_SwitchStream)
			{
				m_SwitchStream->SwitchStream(0);
				m_DevCliSetInfo.m_uiStreamId=0;
				m_SwitchStream->Release();
				m_SwitchStream=NULL;
			}
		}
	}
	else{
		if (NULL!=m_IDeviceClientDecideByVendor)
		{
			ISwitchStream *m_SwitchStream=NULL;
			m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
			if (NULL!=m_SwitchStream)
			{
				m_SwitchStream->SwitchStream(1);
				m_DevCliSetInfo.m_uiStreamId=1;
				m_SwitchStream->Release();
				m_SwitchStream=NULL;
			}
		}
	}
}

void QSubView::RecordState( QVariantMap evMap )
{
	if (_manageWidget->GetRecordItem()!=NULL)
	{
		if (evMap.value("RecordState").toBool()==true)
		{
			emit RecordStateSignals(true);
		}
		else{
			emit RecordStateSignals(false);
		}
	}
}

void QSubView::paintEventNoVideo( QPaintEvent * e)
{
	if (m_CurrentState==STATUS_DISCONNECTED||m_CurrentState==STATUS_DISCONNECTING)
	{
		Q_UNUSED(e);
		QPainter p(this);
		QString image;
		QColor LineColor;
		QColor LineCurColor;
		QColor FontColor;
		int FontSize;
		QString FontFamily;

		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
		QSettings IniFile(path, QSettings::IniFormat, 0);

		image = IniFile.value("background/background-image", QVariant("")).toString();
		LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());
		LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
		FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());
		FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
		FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

		QRect rcClient = contentsRect();
		this->geometry().center();
		QPixmap pix;
		QString PixPaht = sAppPath + image;
		pix.load(PixPaht);

		pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		//背景
		p.drawPixmap(rcClient,pix);
		//边框
		QPen pen = QPen(LineColor);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(rcClient);
		//焦点

		if (m_bIsFocus)
		{
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			rcClient.getCoords(&x, &y, &width, &height);
			pen.setWidth(2);
			pen.setColor(LineCurColor);
			p.setPen(pen);
			p.drawRect(QRectF(x + 2,y + 2,width - 2, height - 2));
		}
		else
		{
			p.drawRect(rcClient);
		}
		//
		int awidth=0;
		int bheight=0;
		int ax=0;
		int ay=0;

		rcClient.getCoords(&ax, &ay, &awidth, &bheight);
		int aFontSize=10;
		int aw=400;
		//	int ah=300;
		aFontSize=awidth*FontSize/(aw);
		QFont font(FontFamily, aFontSize, QFont::Bold);

		p.setFont(font);

		pen.setColor(FontColor);

		p.setPen(pen);

		p.drawText(rcClient, Qt::AlignCenter, "No Video");
	}
	

}

void QSubView::paintEventConnecting( QPaintEvent * e)
{
	if (m_CurrentState==STATUS_CONNECTING)
	{
		Q_UNUSED(e);
		QPainter p(this);
		QString image;
		QColor LineColor;
		QColor LineCurColor;
		QColor FontColor;
		int FontSize;
		QString FontFamily;

		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
		QSettings IniFile(path, QSettings::IniFormat, 0);

		image = IniFile.value("background/background-image", QVariant("")).toString();
		LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());
		LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
		FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());
		FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
		FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

		QRect rcClient = contentsRect();
		this->geometry().center();
		QPixmap pix;
		QString PixPaht = sAppPath + image;
		pix.load(PixPaht);

		pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		//背景
		p.drawPixmap(rcClient,pix);
		//边框
		QPen pen = QPen(LineColor);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(rcClient);
		//焦点

		if (m_bIsFocus)
		{
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			rcClient.getCoords(&x, &y, &width, &height);
			pen.setWidth(5);
			pen.setColor(LineCurColor);
			p.setPen(pen);
			p.drawRect(QRectF(x + 2,y + 2,width - 2, height - 2));
		}
		else
		{
			p.drawRect(rcClient);
		}
		//
		int awidth=0;
		int bheight=0;
		int ax=0;
		int ay=0;

		rcClient.getCoords(&ax, &ay, &awidth, &bheight);
		int aFontSize=10;
		int aw=400;
		//	int ah=300;
		aFontSize=awidth*FontSize/(aw);
		QFont font(FontFamily, aFontSize, QFont::Bold);

		p.setFont(font);

		pen.setColor(FontColor);

		p.setPen(pen);

		QString m_text;
		QPoint dev_position=this->geometry().topLeft();
		if (m_CountConnecting==4)
		{
			m_CountConnecting=0;
		}
		for (int i=0;i<m_CountConnecting;i++)
		{
			m_text+="..";
		}
		m_CountConnecting++;
		p.drawText(rcClient, Qt::AlignCenter, m_text);
	}
}

void QSubView::paintEventCache( QPaintEvent *e )
{
	if (m_CurrentState==STATUS_CONNECTED)
	{
		Q_UNUSED(e);
		QPainter p(this);
		QString image;
		QColor LineColor;
		QColor LineCurColor;
		QColor FontColor;
		int FontSize;
		QString FontFamily;

		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
		QSettings IniFile(path, QSettings::IniFormat, 0);

		image = IniFile.value("background/background-image", QVariant("")).toString();
		LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());
		LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
		FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());
		FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
		FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

		QRect rcClient = contentsRect();
		this->geometry().center();
		QPixmap pix;
		QString PixPaht = sAppPath + image;
		pix.load(PixPaht);

		pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		//背景
		QPixmap m_cacheImage=_cacheBackImage.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		p.drawPixmap(rcClient,m_cacheImage);
		//边框
		QPen pen = QPen(LineColor);
		pen.setWidth(2);
		p.setPen(pen);

		p.drawRect(rcClient);
		//焦点

		if (m_bIsFocus)
		{
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			rcClient.getCoords(&x, &y, &width, &height);
			pen.setWidth(5);
			pen.setColor(LineCurColor);
			p.setPen(pen);
			p.drawRect(QRectF(x,y,width, height));
			/*p.drawRect(QRectF(x + 2,y + 2,width - 2, height - 2));*/
		}
		else
		{
			p.drawRect(rcClient);
		}
	}
}

void QSubView::saveCacheImage()
{
	if (m_CurrentState==STATUS_CONNECTED)
	{
		_cacheBackImage=QPixmap::grabWindow(this->winId(),0,0,this->width(),this->height());
	}
	
}

int QSubView::liveStreamRequire( int nChannel,int nStream,bool bOpen )
{
	IDeviceClient *mLiveStreamRequire=NULL;
	if (m_IDeviceClientDecideByVendor!=NULL)
	{
		m_IDeviceClientDecideByVendor->QueryInterface(IID_IDeviceClient,(void**)&mLiveStreamRequire);
	}
	if (NULL!=mLiveStreamRequire)
	{
		mLiveStreamRequire->liveStreamRequire(nChannel,nStream,bOpen);
		mLiveStreamRequire->Release();
		mLiveStreamRequire=NULL;
	}


	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		ISwitchStream *m_SwitchStream=NULL;
		m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
		if (NULL!=m_SwitchStream)
		{
			m_SwitchStream->SwitchStream(nStream);
			m_SwitchStream->Release();
			m_SwitchStream=NULL;
		}
	}
	return 0;
}

int QSubView::GetDeviceInfo( int chlId )
{
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&pChannelManager);
	if (pChannelManager!=NULL)
	{
		QVariantMap channelInfo=pChannelManager->GetChannelInfo(chlId);
		m_DevCliSetInfo.m_uiStreamId=channelInfo.value("stream").toInt();
		m_DevCliSetInfo.m_uiChannelId=channelInfo.value("number").toInt();
		m_DevCliSetInfo.m_sCameraname=channelInfo.value("name").toString();
		int dev_id=channelInfo.value("dev_id").toInt();
		pChannelManager->Release();
		IDeviceManager *pDeviceManager=NULL;
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void **)&pDeviceManager);
		if (pDeviceManager!=NULL)
		{
			QVariantMap deviceInfo=pDeviceManager->GetDeviceInfo(dev_id);
			m_DevCliSetInfo.m_sVendor=deviceInfo.value("vendor").toString();
			m_DevCliSetInfo.m_sPassword=deviceInfo.value("password").toString();
			m_DevCliSetInfo.m_sUsername=deviceInfo.value("username").toString();
			m_DevCliSetInfo.m_sEseeId=deviceInfo.value("eseeid").toString();
			m_DevCliSetInfo.m_sAddress=deviceInfo.value("address").toString();
			m_DevCliSetInfo.m_uiPort=deviceInfo.value("port").toInt();
			m_DevCliSetInfo.m_uiChannelIdInDataBase=chlId;
			m_DevCliSetInfo.m_sDeviceName=deviceInfo.value("name").toString();
			pDeviceManager->Release();
			return 0;
		}
	}
	return 1;
}

QVariantMap QSubView::GetWindowInfo()
{
	QVariantMap windowInfo;
	windowInfo.insert("focus",m_bIsFocus);
	windowInfo.insert("currentState",m_CurrentState);
	windowInfo.insert("chlId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
	return windowInfo;
}

QVariantMap QSubView::ScreenShot()
{
	_ScreenShotImage=QPixmap::grabWindow(this->winId(),0,0,this->width(),this->height());
	QString dir=QCoreApplication::applicationDirPath();
	dir.append("/temp");
	QDir temp;
	bool exist=temp.exists(dir);
	if (exist==false)
	{
		temp.mkdir(dir);
	}
	QDateTime mtime=QDateTime::currentDateTime();
	uint mutime=mtime.toTime_t();
	QString imageName;
	imageName.append(dir);
	imageName+="/";
	imageName+=QString::number(mutime);
	imageName+=".jpg";
	_ScreenShotImage.save(imageName);
	QVariantMap item;
	item.insert("imageName",QString::number(mutime).append(".jpg"));
	item.insert("path",dir);
	return item;
}

int QSubView::SwitchStream( int chlId)
{
	int preStream=m_DevCliSetInfo.m_uiStreamId;
	GetDeviceInfo(chlId);
	if (chlId==m_DevCliSetInfo.m_uiChannelIdInDataBase&&m_CurrentState==STATUS_CONNECTED&&preStream!=m_DevCliSetInfo.m_uiStreamId)
	{
		liveStreamRequire(m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,true);
		return 0;
	}else{
		return 1;
	}
}

void QSubView::initDeviceInfo()
{
	m_DevCliSetInfo.m_sAddress="";
	m_DevCliSetInfo.m_sCameraname="";
	m_DevCliSetInfo.m_sEseeId="";
	m_DevCliSetInfo.m_sPassword="";
	m_DevCliSetInfo.m_sUsername="";
	m_DevCliSetInfo.m_sVendor="";
	m_DevCliSetInfo.m_uiChannelId=-1;
	m_DevCliSetInfo.m_uiChannelIdInDataBase=-1;
	m_DevCliSetInfo.m_uiPort=-1;
	m_DevCliSetInfo.m_uiStreamId=-1;
}

void QSubView::ResetState()
{
	//设置ipc同步
	//设置计划录像同步
	m_RecordFlushTime=0;
}

void QSubView::changeEvent( QEvent * e)
{
	if (e->type()==QEvent::LanguageChange)
	{
		translateUi();
	}
}

void QSubView::translateUi()
{
	if (m_QActionCloseView!=NULL)
	{
		m_QActionCloseView->setText(tr("Close Preview"));
	}
	if (m_QActionSwitchStream!=NULL)
	{
		m_QActionSwitchStream->setText(tr("Switch Stream"));
	}
}

void QSubView::LoadLanguage(QString label)
{
	if (_translator!=NULL)
	{
		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/LocalSetting";
		_translator->load(GetlanguageLable(label),path);
	}
}
QString QSubView::GetlanguageLable(QString label)
{
	QString sAppPath=QCoreApplication::applicationDirPath();
	sAppPath+="/LocalSetting";
	QDomDocument ConFile;
	QFile *file=new QFile(sAppPath+"/language.xml");
	file->open(QIODevice::ReadOnly);
	ConFile.setContent(file);
	QDomNode clsidNode = ConFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	QString sFileName="en_GB";
	for (int n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString slanguage = item.toElement().attribute("name");
		if(slanguage == label){
			sFileName =item.toElement().attribute("file");
			break;
		}
	}
	return sFileName;
}

void QSubView::SaveToDatobase()
{
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		pChannelManager->ModifyChannelStream(m_DevCliSetInfo.m_uiChannelIdInDataBase,m_DevCliSetInfo.m_uiStreamId);
		pChannelManager->Release();
		pChannelManager=NULL;
	}
}

int QSubView::OpenPTZ( int nCmd, int nSpeed )
{
	if (NULL == m_IDeviceClientDecideByVendor)
	{
		return 1;
	}
	m_IDeviceClientDecideByVendor->QueryInterface(IID_IPTZControl, (void**)&m_pPTZControl);
	if (NULL == m_pPTZControl)
	{
		return 1;
	}

	int nRet = 1;
	int nChl = m_DevCliSetInfo.m_uiChannelId;
	switch (nCmd)
	{
		case 0:
			nRet = m_pPTZControl->ControlPTZUp(nChl, nSpeed);
			break;
		case 1:
			nRet = m_pPTZControl->ControlPTZDown(nChl, nSpeed);
			break;
		case 2:
			nRet = m_pPTZControl->ControlPTZLeft(nChl, nSpeed);
			break;
		case 3:
			nRet = m_pPTZControl->ControlPTZRight(nChl, nSpeed);
			break;
		case 4:
			{
				if (!m_bIsPTZAutoOpened)
				{
					m_bIsPTZAutoOpened = true;
					nRet = m_pPTZControl->ControlPTZAuto(nChl, true);
				}
				else
				{
					m_bIsPTZAutoOpened = false;
					nRet = m_pPTZControl->ControlPTZAuto(nChl, false);
				}
				break;
			}
		case 5:
			nRet = m_pPTZControl->ControlPTZFocusFar(nChl, nSpeed);
			break;
		case 6:
			nRet = m_pPTZControl->ControlPTZFocusNear(nChl, nSpeed);
			break;
		case 7:
			nRet = m_pPTZControl->ControlPTZZoomIn(nChl, nSpeed);
			break;
		case 8:
			nRet = m_pPTZControl->ControlPTZZoomOut(nChl, nSpeed);
			break;
		case 9:
			nRet = m_pPTZControl->ControlPTZIrisOpen(nChl, nSpeed);
			break;
		case 10:
			nRet = m_pPTZControl->ControlPTZIrisClose(nChl, nSpeed);
			break;
		default:
			break;
	}
	return nRet;
}

int QSubView::ClosePTZ( int nCmd )
{
	if (NULL == m_pPTZControl)
	{
		return 1;
	}
	int nRet = 1;
	if (4 != nCmd)
	{
		nRet = m_pPTZControl->ControlPTZStop(m_DevCliSetInfo.m_uiChannelId, nCmd);
	}
	return nRet;
}

